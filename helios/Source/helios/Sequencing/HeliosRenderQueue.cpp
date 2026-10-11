// Copyright Epic Games, Inc. All Rights Reserved.

#include "Sequencing/HeliosRenderQueue.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "LevelSequence.h"
#include "MovieScene.h"
#include "MovieSceneObjectBindingID.h"
#include "Sections/MovieSceneCameraCutSection.h"
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "MoviePipelineQueueEngineSubsystem.h"
#include "MoviePipelineQueue.h"
#include "MoviePipelineInProcessExecutor.h"
#include "MoviePipelinePrimaryConfig.h"
#include "MoviePipelineOutputSetting.h"
#include "MoviePipelineDeferredPasses.h"
#include "MoviePipelineImageSequenceOutput.h"
#include "MoviePipelineSetting.h"
#include "Misc/FrameRate.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeliosRenderQueue, Log, All);

namespace {
const FString HeliosJobPrefix = TEXT("HeliosExport_");

void RemoveHeliosJobs(UMoviePipelineQueue* Queue) {
  for (UMoviePipelineExecutorJob* Job : Queue->GetJobs()) {
    if (Job && Job->JobName.StartsWith(HeliosJobPrefix)) {
      Queue->DeleteJob(Job);
    }
  }
}
}  // namespace

bool FHeliosRenderQueue::QueueCameraExports(
    UWorld& World, ULevelSequence* SourceSequence,
    const FHeliosScenarioConfig& Scenario,
    const TMap<FString, FGuid>& CameraBindings,
    TArray<TObjectPtr<ULevelSequence>>& OutSequences) {
  if (!SourceSequence || !GEngine) {
    UE_LOG(LogHeliosRenderQueue, Error,
           TEXT("Helios: Movie Render Queue needs an engine and sequence"));
    return false;
  }

  UMoviePipelineQueueEngineSubsystem* QueueSubsystem =
      GEngine->GetEngineSubsystem<UMoviePipelineQueueEngineSubsystem>();
  if (!QueueSubsystem || QueueSubsystem->IsRendering()) {
    UE_LOG(
        LogHeliosRenderQueue, Warning,
        TEXT("Helios: Movie Render Queue is unavailable or already rendering"));
    return false;
  }

  TArray<const FHeliosCameraConfig*> EnabledCameras;
  for (const FHeliosCameraConfig& Camera : Scenario.Cameras) {
    if (Camera.Export.bEnabled &&
        Camera.Export.Mode != EHeliosCameraExportMode::None) {
      EnabledCameras.Add(&Camera);
    }
  }

  if (EnabledCameras.IsEmpty()) {
    UE_LOG(LogHeliosRenderQueue, Warning,
           TEXT("Helios: no camera exports are enabled in the scenario"));
    return false;
  }

  UMoviePipelineQueue* Queue = QueueSubsystem->GetQueue();
  for (UMoviePipelineExecutorJob* Job : Queue->GetJobs()) {
    if (Job && !Job->JobName.StartsWith(HeliosJobPrefix)) {
      UE_LOG(LogHeliosRenderQueue, Warning,
             TEXT("Helios: refusing to replace an unrelated Movie Render Queue "
                  "job"));
      return false;
    }
  }
  // Replace only jobs from a previous Helios export; leave other queue entries
  // untouched.
  RemoveHeliosJobs(Queue);
  OutSequences.Reset();

  for (const FHeliosCameraConfig* Camera : EnabledCameras) {
    const double ScaledFrameRate =
        static_cast<double>(Camera->Export.FrameRate) * 1000.0;
    if (Camera->Export.ResolutionX <= 0 || Camera->Export.ResolutionY <= 0 ||
        !FMath::IsFinite(Camera->Export.FrameRate) || ScaledFrameRate < 1.0 ||
        ScaledFrameRate > MAX_int32) {
      UE_LOG(LogHeliosRenderQueue, Error,
             TEXT("Helios: invalid resolution or frame rate for camera '%s'"),
             *Camera->Id);
      RemoveHeliosJobs(Queue);
      OutSequences.Reset();
      return false;
    }

    const FGuid* CameraBinding = CameraBindings.Find(Camera->Id);
    if (!CameraBinding || !CameraBinding->IsValid()) {
      UE_LOG(LogHeliosRenderQueue, Error,
             TEXT("Helios: no sequence binding found for camera '%s'"),
             *Camera->Id);
      RemoveHeliosJobs(Queue);
      OutSequences.Reset();
      return false;
    }

    ULevelSequence* CameraSequence =
        CreateCameraSequence(SourceSequence, Camera->Id, *CameraBinding);
    if (!CameraSequence) {
      RemoveHeliosJobs(Queue);
      OutSequences.Reset();
      return false;
    }

    OutSequences.Add(CameraSequence);
    UMoviePipelineExecutorJob* Job =
        Queue->AllocateNewJob(UMoviePipelineExecutorJob::StaticClass());
    Job->SetSequence(FSoftObjectPath(CameraSequence));
    Job->Map = FSoftObjectPath(&World);
    Job->JobName = HeliosJobPrefix + Camera->Id;

    UMoviePipelinePrimaryConfig* Config = Job->GetConfiguration();
    if (!Config) {
      UE_LOG(LogHeliosRenderQueue, Error,
             TEXT("Helios: failed to create render settings for camera '%s'"),
             *Camera->Id);
      RemoveHeliosJobs(Queue);
      OutSequences.Reset();
      return false;
    }

    UMoviePipelineOutputSetting* Output =
        Cast<UMoviePipelineOutputSetting>(Config->FindOrAddSettingByClass(
            UMoviePipelineOutputSetting::StaticClass()));
    if (!Output) {
      UE_LOG(LogHeliosRenderQueue, Error,
             TEXT("Helios: failed to create output settings for camera '%s'"),
             *Camera->Id);
      RemoveHeliosJobs(Queue);
      OutSequences.Reset();
      return false;
    }
    Output->OutputDirectory.Path =
        Camera->Export.OutputDirectory.IsEmpty()
            ? FPaths::ProjectSavedDir() / TEXT("Renders")
            : (FPaths::IsRelative(Camera->Export.OutputDirectory)
                   ? FPaths::ProjectDir() / Camera->Export.OutputDirectory
                   : Camera->Export.OutputDirectory);
    const FString OutputName = Camera->Export.OutputName.IsEmpty()
                                   ? Camera->Id
                                   : Camera->Export.OutputName;
    Output->FileNameFormat =
        Camera->Export.Mode == EHeliosCameraExportMode::ImageSequence
            ? OutputName + TEXT(".{frame_number}")
            : OutputName;
    Output->OutputResolution =
        FIntPoint(Camera->Export.ResolutionX, Camera->Export.ResolutionY);
    Output->bUseCustomFrameRate = true;
    // A 1000 denominator retains common fractional frame rates such as 29.97
    // fps.
    Output->OutputFrameRate =
        FFrameRate(FMath::RoundToInt(ScaledFrameRate), 1000);

    if (!Config->FindOrAddSettingByClass(
            UMoviePipelineDeferredPassBase::StaticClass())) {
      UE_LOG(LogHeliosRenderQueue, Error,
             TEXT("Helios: failed to create render pass for camera '%s'"),
             *Camera->Id);
      RemoveHeliosJobs(Queue);
      OutSequences.Reset();
      return false;
    }
    if (Camera->Export.Mode == EHeliosCameraExportMode::ImageSequence) {
      if (!Config->FindOrAddSettingByClass(
              UMoviePipelineImageSequenceOutput_PNG::StaticClass())) {
        UE_LOG(LogHeliosRenderQueue, Error,
               TEXT("Helios: failed to create PNG output for camera '%s'"),
               *Camera->Id);
        RemoveHeliosJobs(Queue);
        OutSequences.Reset();
        return false;
      }
    } else {
      // Resolve the Epic encoder class by path because its public header
      // includes a private codec header.
      UClass* MP4OutputClass = StaticLoadClass(
          UMoviePipelineSetting::StaticClass(), nullptr,
          TEXT("/Script/"
               "MovieRenderPipelineMP4Encoder.MoviePipelineMP4EncoderOutput"));
      if (!MP4OutputClass) {
        UE_LOG(
            LogHeliosRenderQueue, Error,
            TEXT("Helios: built-in MP4 encoder is unavailable for camera '%s'"),
            *Camera->Id);
        RemoveHeliosJobs(Queue);
        OutSequences.Reset();
        return false;
      }
      if (!Config->FindOrAddSettingByClass(MP4OutputClass)) {
        UE_LOG(LogHeliosRenderQueue, Error,
               TEXT("Helios: failed to create MP4 output for camera '%s'"),
               *Camera->Id);
        RemoveHeliosJobs(Queue);
        OutSequences.Reset();
        return false;
      }
    }

    UE_LOG(LogHeliosRenderQueue, Log,
           TEXT("Helios: queued %s export for camera '%s' to '%s'"),
           Camera->Export.Mode == EHeliosCameraExportMode::Video
               ? TEXT("MP4")
               : TEXT("PNG sequence"),
           *Camera->Id, *Output->OutputDirectory.Path);
  }

  // Keep execution in the active world so runtime-spawned actors remain bound.
  UMoviePipelineInProcessExecutor* Executor =
      NewObject<UMoviePipelineInProcessExecutor>(QueueSubsystem);
  Executor->bUseCurrentLevel = true;
  QueueSubsystem->RenderQueueWithExecutorInstance(Executor);
  return true;
}

ULevelSequence* FHeliosRenderQueue::CreateCameraSequence(
    ULevelSequence* SourceSequence, const FString& CameraId,
    const FGuid& CameraBinding) {
  const FString SequenceName =
      FString::Printf(TEXT("HeliosRender_%s"), *CameraId);
  ULevelSequence* CameraSequence = DuplicateObject<ULevelSequence>(
      SourceSequence, GetTransientPackage(), FName(*SequenceName));
  if (!CameraSequence) {
    UE_LOG(LogHeliosRenderQueue, Error,
           TEXT("Helios: failed to duplicate sequence for camera '%s'"),
           *CameraId);
    return nullptr;
  }

  UMovieScene* MovieScene = CameraSequence->GetMovieScene();
  UMovieSceneCameraCutTrack* CameraCutTrack = Cast<UMovieSceneCameraCutTrack>(
      MovieScene->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass()));
  if (!CameraCutTrack) {
    UE_LOG(LogHeliosRenderQueue, Error,
           TEXT("Helios: failed to create camera cut track for '%s'"),
           *CameraId);
    return nullptr;
  }

  UMovieSceneCameraCutSection* Cut = CameraCutTrack->AddNewCameraCut(
      FMovieSceneObjectBindingID(
          UE::MovieScene::FRelativeObjectBindingID(CameraBinding)),
      MovieScene->GetPlaybackRange().GetLowerBoundValue());
  Cut->SetRange(MovieScene->GetPlaybackRange());
  return CameraSequence;
}
