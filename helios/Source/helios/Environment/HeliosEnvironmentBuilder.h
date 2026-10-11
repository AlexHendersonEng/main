// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Config/HeliosScenarioTypes.h"

class UWorld;

/** Adds scenario-dependent lighting, atmosphere, ground, and water visuals. */
class FHeliosEnvironmentBuilder {
 public:
  static void Build(UWorld* World, const FHeliosScenarioConfig& Scenario);
};
