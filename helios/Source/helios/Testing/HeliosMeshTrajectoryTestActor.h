// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Trajectory/HeliosTrajectoryTypes.h"
#include "HeliosMeshTrajectoryTestActor.generated.h"

class UProceduralMeshComponent;

/**
 * Checkpoint-2 verification actor: loads a mesh (.obj/.stl) and a trajectory
 * (.csv) from disk and drives the mesh's transform from the trajectory every
 * tick. Not part of the final vehicle framework (that arrives in Checkpoint 3)
 * - this just proves the parsers work end-to-end in PIE.
 */
UCLASS()
class HELIOS_API AHeliosMeshTrajectoryTestActor : public AActor {
  GENERATED_BODY()

 public:
  AHeliosMeshTrajectoryTestActor();

  /** Absolute or project-relative path to a .obj or .stl file. */
  UPROPERTY(EditAnywhere, Category = "Helios|Test")
  FString ModelPath;

  /** Absolute or project-relative path to a trajectory .csv file. */
  UPROPERTY(EditAnywhere, Category = "Helios|Test")
  FString TrajectoryCsvPath;

  /** Playback speed multiplier for the driven trajectory. */
  UPROPERTY(EditAnywhere, Category = "Helios|Test")
  float PlaybackSpeed = 1.f;

 protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaSeconds) override;

 private:
  UPROPERTY(VisibleAnywhere, Category = "Helios|Test")
  TObjectPtr<UProceduralMeshComponent> MeshComponent;

  TArray<FHeliosTrajectorySample> TrajectorySamples;
  float ElapsedTime = 0.f;
};
