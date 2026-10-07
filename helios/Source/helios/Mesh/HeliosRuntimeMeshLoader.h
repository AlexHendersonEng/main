// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeliosMeshData.h"

/**
 * Loads vehicle visual meshes at runtime directly from .obj or .stl files (no
 * editor import step). Output is in Unreal units (cm): source coordinates are
 * assumed to be in meters, Z-up, and are simply scaled by 100 - no axis
 * remapping is performed. Use a vehicle's "scale" config field for any further
 * adjustment.
 */
class HELIOS_API FHeliosRuntimeMeshLoader {
 public:
  /** Loads a .obj or .stl file (chosen by extension) into flat mesh data. */
  static bool LoadMeshFromFile(const FString& FilePath,
                               FHeliosMeshData& OutMeshData);

  /** Applies loaded mesh data to a ProceduralMeshComponent section, computing
   * tangents and enabling collision. */
  static void ApplyToComponent(UProceduralMeshComponent* Component,
                               const FHeliosMeshData& MeshData,
                               int32 SectionIndex = 0,
                               bool bCreateCollision = true);

 private:
  static bool LoadObj(const FString& FilePath, FHeliosMeshData& OutMeshData);
  static bool LoadStl(const FString& FilePath, FHeliosMeshData& OutMeshData);
  static bool LoadStlAscii(const TArray<FString>& Lines,
                           FHeliosMeshData& OutMeshData);
  static bool LoadStlBinary(const TArray<uint8>& Bytes,
                            FHeliosMeshData& OutMeshData);
};
