// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HeliosPlaybackSubsystem.generated.h"

class ULevelSequence;
class ULevelSequencePlayer;
class ALevelSequenceActor;
class AHeliosVehicleActor;
class ACameraActor;

/**
 * Builds the scenario (vehicles/cameras/sequence) when the world starts playing
 * and owns the ULevelSequencePlayer that drives live preview. The playback UI
 * (Checkpoint 4) will call the Play/Pause/SetPlaybackPosition wrappers below.
 */
UCLASS()
class HELIOS_API UHeliosPlaybackSubsystem : public UWorldSubsystem {
  GENERATED_BODY()

 public:
  virtual void OnWorldBeginPlay(UWorld& InWorld) override;

  UFUNCTION(BlueprintCallable, Category = "Helios|Playback")
  void Play();

  UFUNCTION(BlueprintCallable, Category = "Helios|Playback")
  void Pause();

  /** Scrubs to an absolute time in seconds from the start of the sequence. */
  UFUNCTION(BlueprintCallable, Category = "Helios|Playback")
  void SetPlaybackPositionSeconds(float Seconds);

  UFUNCTION(BlueprintCallable, Category = "Helios|Playback")
  void SetPlayRate(float Rate);

 private:
  UPROPERTY()
  TObjectPtr<ULevelSequence> Sequence;

  UPROPERTY()
  TObjectPtr<ULevelSequencePlayer> SequencePlayer;

  UPROPERTY()
  TObjectPtr<ALevelSequenceActor> SequenceActor;

  UPROPERTY()
  TArray<TObjectPtr<AHeliosVehicleActor>> SpawnedVehicles;

  UPROPERTY()
  TArray<TObjectPtr<ACameraActor>> SpawnedCameras;
};
