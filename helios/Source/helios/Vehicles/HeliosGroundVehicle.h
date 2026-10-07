// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeliosVehicleActor.h"
#include "HeliosGroundVehicle.generated.h"

/** Ground vehicle; extension point for ground-contact/suspension visuals in
 * later checkpoints. */
UCLASS()
class HELIOS_API AHeliosGroundVehicle : public AHeliosVehicleActor {
  GENERATED_BODY()

 public:
  AHeliosGroundVehicle() { VehicleType = EHeliosVehicleType::GroundVehicle; }
};
