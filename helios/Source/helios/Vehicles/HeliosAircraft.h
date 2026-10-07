// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeliosVehicleActor.h"
#include "HeliosAircraft.generated.h"

/** Aircraft vehicle; extension point for flight-specific visuals/behaviour in
 * later checkpoints. */
UCLASS()
class HELIOS_API AHeliosAircraft : public AHeliosVehicleActor {
  GENERATED_BODY()

 public:
  AHeliosAircraft() { VehicleType = EHeliosVehicleType::Aircraft; }
};
