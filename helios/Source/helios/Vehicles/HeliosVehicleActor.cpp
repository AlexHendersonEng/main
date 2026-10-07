// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeliosVehicleActor.h"

#include "ProceduralMeshComponent.h"
#include "Misc/Paths.h"
#include "Mesh/HeliosRuntimeMeshLoader.h"

AHeliosVehicleActor::AHeliosVehicleActor() {
  PrimaryActorTick.bCanEverTick = false;

  MeshComponent =
      CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
  RootComponent = MeshComponent;
}

void AHeliosVehicleActor::InitializeFromConfig(
    const FHeliosVehicleConfig& Config) {
  VehicleId = Config.Id;
  VehicleType = Config.Type;

  const FString ResolvedPath = FPaths::IsRelative(Config.ModelPath)
                                   ? FPaths::ProjectDir() / Config.ModelPath
                                   : Config.ModelPath;
  FHeliosMeshData MeshData;
  if (FHeliosRuntimeMeshLoader::LoadMeshFromFile(ResolvedPath, MeshData)) {
    FHeliosRuntimeMeshLoader::ApplyToComponent(MeshComponent, MeshData);
  }

  SetActorScale3D(FVector(Config.Scale));
  SetActorHiddenInGame(!Config.bVisible);
}
