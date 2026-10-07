// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeliosMeshTrajectoryTestActor.h"

#include "ProceduralMeshComponent.h"
#include "Misc/Paths.h"
#include "Mesh/HeliosRuntimeMeshLoader.h"
#include "Trajectory/HeliosTrajectoryCsv.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeliosTest, Log, All);

AHeliosMeshTrajectoryTestActor::AHeliosMeshTrajectoryTestActor() {
  PrimaryActorTick.bCanEverTick = true;

  MeshComponent =
      CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
  RootComponent = MeshComponent;
}

void AHeliosMeshTrajectoryTestActor::BeginPlay() {
  Super::BeginPlay();

  // Paths may be relative to the project directory so sample assets can ship
  // inside the repo.
  const FString ResolvedModelPath = FPaths::IsRelative(ModelPath)
                                        ? FPaths::ProjectDir() / ModelPath
                                        : ModelPath;
  const FString ResolvedCsvPath = FPaths::IsRelative(TrajectoryCsvPath)
                                      ? FPaths::ProjectDir() / TrajectoryCsvPath
                                      : TrajectoryCsvPath;

  FHeliosMeshData MeshData;
  if (FHeliosRuntimeMeshLoader::LoadMeshFromFile(ResolvedModelPath, MeshData)) {
    FHeliosRuntimeMeshLoader::ApplyToComponent(MeshComponent, MeshData);
  }

  if (!FHeliosTrajectoryCsv::LoadFromFile(ResolvedCsvPath, TrajectorySamples)) {
    UE_LOG(LogHeliosTest, Warning,
           TEXT("Helios: test actor has no trajectory to play back"));
  }
}

void AHeliosMeshTrajectoryTestActor::Tick(float DeltaSeconds) {
  Super::Tick(DeltaSeconds);

  if (TrajectorySamples.IsEmpty()) {
    return;
  }

  ElapsedTime += DeltaSeconds * PlaybackSpeed;
  const FTransform Sampled =
      FHeliosTrajectoryCsv::SampleAtTime(TrajectorySamples, ElapsedTime);
  SetActorTransform(Sampled);
}
