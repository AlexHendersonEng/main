// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeliosTrajectoryTypes.h"

/**
 * Loads trajectory/attitude CSV files (columns: time,x,y,z,roll,pitch,yaw) and
 * samples them with linear interpolation between timestamps.
 */
class HELIOS_API FHeliosTrajectoryCsv {
 public:
  /** Parses a CSV file into a time-ordered sample array. An optional header row
   * is auto-detected and skipped. */
  static bool LoadFromFile(const FString& FilePath,
                           TArray<FHeliosTrajectorySample>& OutSamples);

  /** Linearly interpolates position and slerps rotation between the two samples
   * bracketing Time; clamps at the ends. */
  static FTransform SampleAtTime(const TArray<FHeliosTrajectorySample>& Samples,
                                 float Time);
};
