// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeliosVehicleActor.h"
#include "HeliosBoat.generated.h"

/** Surface vessel; extension point for water-surface interaction visuals in
 * later checkpoints. */
UCLASS()
class HELIOS_API AHeliosBoat : public AHeliosVehicleActor {
  GENERATED_BODY()

 public:
  AHeliosBoat() { VehicleType = EHeliosVehicleType::Boat; }
};
