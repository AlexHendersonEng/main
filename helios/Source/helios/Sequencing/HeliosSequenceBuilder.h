// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/FrameNumber.h"
#include "Config/HeliosScenarioTypes.h"

class AHeliosVehicleActor;
class ACameraActor;
class ULevelSequence;
class UMovieScene3DTransformSection;

/** Everything spawned/built for one scenario: the generated sequence plus the
 * actors it drives. */
struct FHeliosBuiltScenario {
  TObjectPtr<ULevelSequence> Sequence;
  TArray<TObjectPtr<AHeliosVehicleActor>> Vehicles;
  TArray<TObjectPtr<ACameraActor>> Cameras;
};

/**
 * Spawns vehicle/camera actors from a scenario config and builds a
 * LevelSequence whose transform tracks are keyframed from each vehicle's
 * trajectory CSV. The sequence is the single source of truth for playback -
 * live preview and Movie Render Queue export both read from it.
 */
class FHeliosSequenceBuilder {
 public:
  static FHeliosBuiltScenario BuildFromScenario(
      UWorld* World, const FHeliosScenarioConfig& Scenario);

 private:
  static AHeliosVehicleActor* SpawnVehicle(UWorld* World,
                                           const FHeliosVehicleConfig& Config);
  static ACameraActor* SpawnCamera(UWorld* World,
                                   const FHeliosCameraConfig& Config,
                                   AHeliosVehicleActor* AttachParent);

  /** Possessable binding is created manually (AddPossessable +
   * BindPossessableObject) since ULevelSequence::FindOrAddBinding is protected
   * in this engine version. */
  static FGuid AddPossessableBinding(ULevelSequence* Sequence, UWorld* World,
                                     UObject* BoundObject);

  /** Binds BoundObject into the sequence and keyframes a 3D transform track
   * from the trajectory samples; returns the last keyframed frame (0 if the CSV
   * couldn't be loaded). */
  static FFrameNumber AddTrajectoryTrack(ULevelSequence* Sequence,
                                         UWorld* World, UObject* BoundObject,
                                         const FString& TrajectoryCsvPath);
};
