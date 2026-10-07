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
    Play();
  }
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
