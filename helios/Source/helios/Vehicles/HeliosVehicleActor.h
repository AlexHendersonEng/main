// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Config/HeliosScenarioTypes.h"
#include "HeliosVehicleActor.generated.h"

class UProceduralMeshComponent;

/**
 * Base class for all Helios vehicles (aircraft/ground/boat/underwater). Holds
 * the runtime-loaded mesh; its transform is driven externally by a
 * LevelSequence transform track built from the vehicle's trajectory CSV, not by
 * this actor's own tick.
 */
UCLASS(Abstract)
class HELIOS_API AHeliosVehicleActor : public AActor {
  GENERATED_BODY()

 public:
  AHeliosVehicleActor();

  /** Loads the mesh from Config.ModelPath and applies scale/visibility/id; does
   * not touch the transform. */
  void InitializeFromConfig(const FHeliosVehicleConfig& Config);

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Vehicle")
  EHeliosVehicleType VehicleType = EHeliosVehicleType::Aircraft;

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Vehicle")
  FString VehicleId;

 protected:
  UPROPERTY(VisibleAnywhere, Category = "Helios|Vehicle")
  TObjectPtr<UProceduralMeshComponent> MeshComponent;
};
