// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Config/HeliosScenarioTypes.h"

class ULevelSequence;
class UWorld;

/** Builds and starts one Movie Render Queue job for each enabled camera export.
 */
class FHeliosRenderQueue {
 public:
  static bool QueueCameraExports(
      UWorld& World, ULevelSequence* SourceSequence,
      const FHeliosScenarioConfig& Scenario,
      const TMap<FString, FGuid>& CameraBindings,
      TArray<TObjectPtr<ULevelSequence>>& OutSequences);

 private:
  static ULevelSequence* CreateCameraSequence(ULevelSequence* SourceSequence,
                                              const FString& CameraId,
                                              const FGuid& CameraBinding);
};
