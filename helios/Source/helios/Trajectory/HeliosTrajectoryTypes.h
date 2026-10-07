// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeliosTrajectoryTypes.generated.h"

/** One row of a trajectory CSV: a timestamped position + attitude sample. */
USTRUCT(BlueprintType)
struct FHeliosTrajectorySample {
  GENERATED_BODY()

  /** Seconds from the start of the trajectory. */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Trajectory")
  float Time = 0.f;

  /** World position in meters (as authored in the CSV; callers convert to
   * Unreal units/cm as needed). */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Trajectory")
  FVector Position = FVector::ZeroVector;

  /** Attitude in degrees (roll, pitch, yaw). */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Trajectory")
  FRotator Rotation = FRotator::ZeroRotator;
};
