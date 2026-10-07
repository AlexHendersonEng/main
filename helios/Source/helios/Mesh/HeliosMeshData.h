// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"

/** Flat (non-indexed-sharing) triangle mesh data produced by the OBJ/STL
 * importers, ready for ProceduralMeshComponent. */
struct FHeliosMeshData {
  TArray<FVector> Vertices;
  TArray<int32> Triangles;
  TArray<FVector> Normals;
  TArray<FVector2D> UVs;
  TArray<FProcMeshTangent> Tangents;

  bool IsEmpty() const { return Vertices.IsEmpty() || Triangles.IsEmpty(); }
};
