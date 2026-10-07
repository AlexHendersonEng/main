// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeliosSequenceBuilder.h"

#include "LevelSequence.h"
#include "MovieScene.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Misc/FrameRate.h"
#include "Misc/Paths.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Vehicles/HeliosVehicleActor.h"
#include "Vehicles/HeliosAircraft.h"
#include "Vehicles/HeliosGroundVehicle.h"
#include "Vehicles/HeliosBoat.h"
#include "Vehicles/HeliosUnderwaterVessel.h"
#include "Trajectory/HeliosTrajectoryCsv.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeliosSequence, Log, All);

namespace {
// Trajectory CSVs are authored in meters; the mesh loader applies the same
// scale to geometry.
constexpr float MetersToUnreal = 100.f;
}  // namespace

FHeliosBuiltScenario FHeliosSequenceBuilder::BuildFromScenario(
    UWorld* World, const FHeliosScenarioConfig& Scenario) {
  FHeliosBuiltScenario Result;
  if (!World) {
    return Result;
  }

  // Keep generated bindings scoped to this runtime scenario, not a saved asset.
  ULevelSequence* Sequence =
      NewObject<ULevelSequence>(GetTransientPackage(), NAME_None, RF_Transient);
  Sequence->Initialize();
  Result.Sequence = Sequence;

  // Spawn vehicles first so cameras can attach to them by Id.
  TMap<FString, AHeliosVehicleActor*> VehiclesById;
  FFrameNumber MaxFrame(0);
  for (const FHeliosVehicleConfig& VehicleConfig : Scenario.Vehicles) {
    AHeliosVehicleActor* Vehicle = SpawnVehicle(World, VehicleConfig);
    if (!Vehicle) {
      continue;
    }
    VehiclesById.Add(VehicleConfig.Id, Vehicle);
    Result.Vehicles.Add(Vehicle);
    MaxFrame = FMath::Max(MaxFrame,
                          AddTrajectoryTrack(Sequence, World, Vehicle,
                                             VehicleConfig.TrajectoryCsvPath));
  }

  for (const FHeliosCameraConfig& CameraConfig : Scenario.Cameras) {
    AHeliosVehicleActor* const* AttachParent =
        CameraConfig.AttachedToVehicleId.IsEmpty()
            ? nullptr
            : VehiclesById.Find(CameraConfig.AttachedToVehicleId);
    ACameraActor* Camera = SpawnCamera(World, CameraConfig,
                                       AttachParent ? *AttachParent : nullptr);
    if (Camera) {
      Result.Cameras.Add(Camera);
      // Bind (no keyframes yet) so Movie Render Queue camera-cut setup in
      // Checkpoint 5 has a binding to use.
      AddPossessableBinding(Sequence, World, Camera);
    }
  }

  // Playback range must cover the longest trajectory, else the sequence player
  // has nothing to play.
  const FFrameRate TickResolution =
      Sequence->GetMovieScene()->GetTickResolution();
  Sequence->GetMovieScene()->SetPlaybackRange(
      FFrameNumber(0), MaxFrame.Value > 0
                           ? MaxFrame.Value
                           : TickResolution.AsFrameNumber(1.0).Value);

  UE_LOG(LogHeliosSequence, Log,
         TEXT("Helios: built sequence with %d vehicle(s) and %d camera(s)"),
         Result.Vehicles.Num(), Result.Cameras.Num());
  return Result;
}

AHeliosVehicleActor* FHeliosSequenceBuilder::SpawnVehicle(
    UWorld* World, const FHeliosVehicleConfig& Config) {
  UClass* ActorClass = AHeliosAircraft::StaticClass();
  switch (Config.Type) {
    case EHeliosVehicleType::GroundVehicle:
      ActorClass = AHeliosGroundVehicle::StaticClass();
      break;
    case EHeliosVehicleType::Boat:
      ActorClass = AHeliosBoat::StaticClass();
      break;
    case EHeliosVehicleType::UnderwaterVessel:
      ActorClass = AHeliosUnderwaterVessel::StaticClass();
      break;
    default:
      break;  // Aircraft is the default class above.
  }

  FActorSpawnParameters SpawnParams;
  SpawnParams.Name = FName(*Config.Id);
  AHeliosVehicleActor* Vehicle = World->SpawnActor<AHeliosVehicleActor>(
      ActorClass, FTransform::Identity, SpawnParams);
  if (Vehicle) {
    Vehicle->InitializeFromConfig(Config);
  }
  return Vehicle;
}

ACameraActor* FHeliosSequenceBuilder::SpawnCamera(
    UWorld* World, const FHeliosCameraConfig& Config,
    AHeliosVehicleActor* AttachParent) {
  FActorSpawnParameters SpawnParams;
  SpawnParams.Name = FName(*Config.Id);
  ACameraActor* Camera = World->SpawnActor<ACameraActor>(
      Config.Location, Config.Rotation, SpawnParams);
  if (!Camera) {
    return nullptr;
  }

  Camera->GetCameraComponent()->SetFieldOfView(Config.FieldOfView);

  if (AttachParent) {
    // Plain actor attachment: the camera then moves with its parent for free,
    // no extra keyframing needed.
    Camera->AttachToActor(AttachParent,
                          FAttachmentTransformRules::KeepRelativeTransform);
    Camera->SetActorRelativeLocation(Config.Location);
    Camera->SetActorRelativeRotation(Config.Rotation);
  }

  return Camera;
}

FGuid FHeliosSequenceBuilder::AddPossessableBinding(ULevelSequence* Sequence,
                                                    UWorld* World,
                                                    UObject* BoundObject) {
  const FGuid Guid = Sequence->GetMovieScene()->AddPossessable(
      BoundObject->GetName(), BoundObject->GetClass());
  Sequence->BindPossessableObject(Guid, *BoundObject, World);
  return Guid;
}

FFrameNumber FHeliosSequenceBuilder::AddTrajectoryTrack(
    ULevelSequence* Sequence, UWorld* World, UObject* BoundObject,
    const FString& TrajectoryCsvPath) {
  TArray<FHeliosTrajectorySample> Samples;
  const FString ResolvedPath = FPaths::IsRelative(TrajectoryCsvPath)
                                   ? FPaths::ProjectDir() / TrajectoryCsvPath
                                   : TrajectoryCsvPath;
  if (!FHeliosTrajectoryCsv::LoadFromFile(ResolvedPath, Samples)) {
    // Leave the spawned vehicle unkeyframed when its trajectory cannot be
    // loaded.
    return FFrameNumber(0);
  }
  UMovieScene* MovieScene = Sequence->GetMovieScene();
  const FGuid Guid = AddPossessableBinding(Sequence, World, BoundObject);

  UMovieScene3DTransformTrack* Track =
      MovieScene->AddTrack<UMovieScene3DTransformTrack>(Guid);
  UMovieScene3DTransformSection* Section =
      Cast<UMovieScene3DTransformSection>(Track->CreateNewSection());
  Track->AddSection(*Section);

  const FFrameRate TickResolution = MovieScene->GetTickResolution();
  TArray<FFrameNumber> Times;
  TArray<FMovieSceneDoubleValue> PosX, PosY, PosZ, RotX, RotY, RotZ;
  Times.Reserve(Samples.Num());
  for (const FHeliosTrajectorySample& Sample : Samples) {
    // Sequencer keys use tick-resolution frames; CSV timestamps are seconds.
    Times.Add(TickResolution.AsFrameNumber(Sample.Time));
    PosX.Add(FMovieSceneDoubleValue(Sample.Position.X * MetersToUnreal));
    PosY.Add(FMovieSceneDoubleValue(Sample.Position.Y * MetersToUnreal));
    PosZ.Add(FMovieSceneDoubleValue(Sample.Position.Z * MetersToUnreal));
    RotX.Add(FMovieSceneDoubleValue(Sample.Rotation.Roll));
    RotY.Add(FMovieSceneDoubleValue(Sample.Rotation.Pitch));
    RotZ.Add(FMovieSceneDoubleValue(Sample.Rotation.Yaw));
  }

  TArrayView<FMovieSceneDoubleChannel*> Channels =
      Section->GetChannelProxy().GetChannels<FMovieSceneDoubleChannel>();
  // Channel order is fixed by UMovieScene3DTransformSection: translation XYZ,
  // rotation XYZ, scale XYZ.
  Channels[0]->Set(Times, TArray<FMovieSceneDoubleValue>(PosX));
  Channels[1]->Set(Times, TArray<FMovieSceneDoubleValue>(PosY));
  Channels[2]->Set(Times, TArray<FMovieSceneDoubleValue>(PosZ));
  Channels[3]->Set(Times, TArray<FMovieSceneDoubleValue>(RotX));
  Channels[4]->Set(Times, TArray<FMovieSceneDoubleValue>(RotY));
  Channels[5]->Set(Times, TArray<FMovieSceneDoubleValue>(RotZ));

  // Include the last sample so playback reaches the trajectory endpoint.
  const FFrameNumber LastFrame = Times.Last();
  Section->SetRange(TRange<FFrameNumber>(
      FFrameNumber(0), TRangeBound<FFrameNumber>::Inclusive(LastFrame)));
  return LastFrame;
}
