// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeliosEnvironmentBuilder.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeliosEnvironment, Log, All);

namespace {
constexpr float PlaneScale = 5000.f;
constexpr float GroundDepth = -20.f;

bool HasVehicleType(const FHeliosScenarioConfig& Scenario,
                    EHeliosVehicleType Type) {
  return Scenario.Vehicles.ContainsByPredicate(
      [Type](const FHeliosVehicleConfig& Vehicle) {
        return Vehicle.Type == Type;
      });
}

AStaticMeshActor* SpawnPlane(UWorld* World, const TCHAR* Name, float Z,
                             float Scale, UMaterialInterface* Material) {
  const FTransform Transform(FRotator::ZeroRotator, FVector(0.f, 0.f, Z),
                             FVector(Scale));
  AStaticMeshActor* Plane = World->SpawnActor<AStaticMeshActor>(
      AStaticMeshActor::StaticClass(), Transform);
  if (!Plane) {
    UE_LOG(LogHeliosEnvironment, Error,
           TEXT("Helios: failed to spawn %s environment plane"), Name);
    return nullptr;
  }

  UStaticMeshComponent* Mesh = Plane->GetStaticMeshComponent();
  // Engine primitive meshes keep both environment surfaces dependency-free.
  UStaticMesh* PlaneMesh =
      LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
  if (!PlaneMesh) {
    UE_LOG(LogHeliosEnvironment, Error,
           TEXT("Helios: default plane mesh unavailable for %s"), Name);
    Plane->Destroy();
    return nullptr;
  }
  Mesh->SetStaticMesh(PlaneMesh);
  Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  Mesh->SetCastShadow(false);
  if (Material) {
    Mesh->SetMaterial(0, Material);
  }
  return Plane;
}
}  // namespace

void FHeliosEnvironmentBuilder::Build(UWorld* World,
                                      const FHeliosScenarioConfig& Scenario) {
  if (!World) {
    return;
  }

  if (!World->SpawnActor<ASkyAtmosphere>()) {
    UE_LOG(LogHeliosEnvironment, Error,
           TEXT("Helios: failed to spawn sky atmosphere"));
  }

  ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(
      FVector::ZeroVector, FRotator(-45.f, -35.f, 0.f));
  if (Sun) {
    Sun->GetLightComponent()->SetIntensity(10.f);
  } else {
    UE_LOG(LogHeliosEnvironment, Error,
           TEXT("Helios: failed to spawn directional sunlight"));
  }

  ASkyLight* SkyLight = World->SpawnActor<ASkyLight>();
  if (SkyLight) {
    SkyLight->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    SkyLight->GetLightComponent()->RecaptureSky();
  } else {
    UE_LOG(LogHeliosEnvironment, Error,
           TEXT("Helios: failed to spawn sky light"));
  }

  AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>();
  if (Fog) {
    Fog->GetComponent()->FogDensity = 0.003f;
    Fog->GetComponent()->FogHeightFalloff = 0.2f;
  } else {
    UE_LOG(LogHeliosEnvironment, Error,
           TEXT("Helios: failed to spawn atmospheric fog"));
  }

  int32 GroundCount = 0;
  int32 OceanCount = 0;
  if (HasVehicleType(Scenario, EHeliosVehicleType::GroundVehicle)) {
    GroundCount +=
        SpawnPlane(World, TEXT("Ground"), GroundDepth, PlaneScale, nullptr) ? 1
                                                                            : 0;
  }

  if (HasVehicleType(Scenario, EHeliosVehicleType::Boat) ||
      HasVehicleType(Scenario, EHeliosVehicleType::UnderwaterVessel)) {
    UMaterialInterface* OceanMaterial = LoadObject<UMaterialInterface>(
        nullptr, TEXT("/Water/Materials/WaterSurface/"
                      "Water_Material_Ocean.Water_Material_Ocean"));
    if (!OceanMaterial) {
      UE_LOG(LogHeliosEnvironment, Warning,
             TEXT("Helios: Water plugin ocean material unavailable"));
    }
    OceanCount +=
        SpawnPlane(World, TEXT("Ocean"), 0.f, PlaneScale, OceanMaterial) ? 1
                                                                         : 0;
  }
  UE_LOG(LogHeliosEnvironment, Log,
         TEXT("Helios: environment ready (ground=%d, ocean=%d)"), GroundCount,
         OceanCount);
}
