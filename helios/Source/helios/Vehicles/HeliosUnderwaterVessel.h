// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeliosVehicleActor.h"
#include "HeliosUnderwaterVessel.generated.h"

/** Underwater vessel; extension point for underwater fog/post-process visuals
 * in later checkpoints. */
UCLASS()
class HELIOS_API AHeliosUnderwaterVessel : public AHeliosVehicleActor {
  GENERATED_BODY()

 public:
  AHeliosUnderwaterVessel() {
    VehicleType = EHeliosVehicleType::UnderwaterVessel;
  }
};
