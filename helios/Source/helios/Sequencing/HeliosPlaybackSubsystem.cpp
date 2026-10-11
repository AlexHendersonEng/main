// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeliosPlaybackSubsystem.h"

#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HeliosSequenceBuilder.h"
#include "Config/HeliosScenarioSubsystem.h"
#include "UI/HeliosPlaybackWidget.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Vehicles/HeliosVehicleActor.h"
#include "Sequencing/HeliosRenderQueue.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeliosPlayback, Log, All);

void UHeliosPlaybackSubsystem::OnWorldBeginPlay(UWorld& InWorld) {
  Super::OnWorldBeginPlay(InWorld);

  // The game-instance subsystem owns the parsed scenario used to build playback
  // actors.
  UGameInstance* GameInstance = InWorld.GetGameInstance();
  UHeliosScenarioSubsystem* ScenarioSubsystem =
      GameInstance ? GameInstance->GetSubsystem<UHeliosScenarioSubsystem>()
                   : nullptr;
  if (!ScenarioSubsystem || !ScenarioSubsystem->HasLoadedScenario()) {
    UE_LOG(LogHeliosPlayback, Warning,
           TEXT("Helios: no scenario loaded, skipping playback setup"));
    return;
  }

  const FHeliosBuiltScenario Built = FHeliosSequenceBuilder::BuildFromScenario(
      &InWorld, ScenarioSubsystem->GetScenario());
  // Retain the generated objects for the lifetime of this world subsystem.
  Sequence = Built.Sequence;
  SpawnedVehicles = Built.Vehicles;
  SpawnedCameras = Built.Cameras;
  CameraBindings = Built.CameraBindings;

  if (!Sequence) {
    return;
  }

  FMovieSceneSequencePlaybackSettings PlaybackSettings;
  PlaybackSettings.bAutoPlay = false;  // Start explicitly after setup; the UI
                                       // can use the same controls later.
  // MovieScene uses -1 for infinite looping and 0 for a single pass.
  PlaybackSettings.LoopCount.Value =
      ScenarioSubsystem->GetScenario().Playback.bLoop ? -1 : 0;
  ALevelSequenceActor* SpawnedSequenceActor = nullptr;
  SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
      &InWorld, Sequence, PlaybackSettings, SpawnedSequenceActor);
  SequenceActor = SpawnedSequenceActor;
  if (SequencePlayer) {
    SequencePlayer->SetPlayRate(
        ScenarioSubsystem->GetScenario().Playback.DefaultSpeed);
    CreatePlaybackWidget(InWorld);
    Play();

    // Permit non-interactive runs to launch the same configured render queue.
    if (FParse::Param(FCommandLine::Get(), TEXT("HeliosRender"))) {
      RenderConfiguredCameras();
    }
  }
}

void UHeliosPlaybackSubsystem::CreatePlaybackWidget(UWorld& World) {
  if (World.GetNetMode() == NM_DedicatedServer) {
    return;
  }

  APlayerController* PlayerController = World.GetFirstPlayerController();
  if (!PlayerController) {
    UE_LOG(LogHeliosPlayback, Warning,
           TEXT("Helios: no player controller available for playback UI"));
    return;
  }

  PlaybackWidget = CreateWidget<UHeliosPlaybackWidget>(PlayerController);
  if (!PlaybackWidget) {
    UE_LOG(LogHeliosPlayback, Error,
           TEXT("Helios: failed to create playback UI"));
    return;
  }

  PlaybackWidget->AddToViewport();
  PlayerController->bShowMouseCursor = true;
  FInputModeGameAndUI InputMode;
  InputMode.SetWidgetToFocus(PlaybackWidget->TakeWidget());
  InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
  PlayerController->SetInputMode(InputMode);
  UE_LOG(LogHeliosPlayback, Log, TEXT("Helios: playback UI created"));
}

void UHeliosPlaybackSubsystem::Play() {
  if (SequencePlayer) {
    SequencePlayer->Play();
  }
}

void UHeliosPlaybackSubsystem::Pause() {
  if (SequencePlayer) {
    SequencePlayer->Pause();
  }
}

void UHeliosPlaybackSubsystem::SetPlaybackPositionSeconds(float Seconds) {
  if (SequencePlayer) {
    SequencePlayer->SetPlaybackPosition(FMovieSceneSequencePlaybackParams(
        Seconds, EUpdatePositionMethod::Play));
  }
}

void UHeliosPlaybackSubsystem::SetPlayRate(float Rate) {
  if (SequencePlayer) {
    SequencePlayer->SetPlayRate(Rate);
  }
}

float UHeliosPlaybackSubsystem::GetCurrentTimeSeconds() const {
  return SequencePlayer ? SequencePlayer->GetCurrentTime().AsSeconds() : 0.0f;
}

float UHeliosPlaybackSubsystem::GetDurationSeconds() const {
  return SequencePlayer ? SequencePlayer->GetDuration().AsSeconds() : 0.0f;
}

float UHeliosPlaybackSubsystem::GetPlayRate() const {
  return SequencePlayer ? SequencePlayer->GetPlayRate() : 1.0f;
}

bool UHeliosPlaybackSubsystem::IsPlaying() const {
  return SequencePlayer && SequencePlayer->IsPlaying();
}

void UHeliosPlaybackSubsystem::SetVehicleVisible(const FString& VehicleId,
                                                 bool bVisible) {
  for (AHeliosVehicleActor* Vehicle : SpawnedVehicles) {
    if (Vehicle && Vehicle->VehicleId == VehicleId) {
      Vehicle->SetActorHiddenInGame(!bVisible);
      return;
    }
  }
}

bool UHeliosPlaybackSubsystem::RenderConfiguredCameras() {
  // Export jobs are created on demand so normal preview playback stays
  // lightweight.
  UWorld* World = GetWorld();
  UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
  UHeliosScenarioSubsystem* ScenarioSubsystem =
      GameInstance ? GameInstance->GetSubsystem<UHeliosScenarioSubsystem>()
                   : nullptr;

  if (!World || !ScenarioSubsystem || !ScenarioSubsystem->HasLoadedScenario() ||
      !Sequence) {
    UE_LOG(
        LogHeliosPlayback, Error,
        TEXT("Helios: cannot render without a loaded scenario and sequence"));
    return false;
  }

  const bool bResumePlaybackOnFailure = IsPlaying();
  Pause();  // Prevent the preview player and MRQ from driving the same actors
            // concurrently.
  const bool bQueued = FHeliosRenderQueue::QueueCameraExports(
      *World, Sequence, ScenarioSubsystem->GetScenario(), CameraBindings,
      ExportSequences);

  if (!bQueued && bResumePlaybackOnFailure) {
    Play();
  }

  return bQueued;
}
