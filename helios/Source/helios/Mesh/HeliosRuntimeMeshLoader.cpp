// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeliosRuntimeMeshLoader.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "KismetProceduralMeshLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeliosMesh, Log, All);

namespace {
// Meters (source files) -> Unreal centimeters.
constexpr float MetersToUnreal = 100.f;
}  // namespace

bool FHeliosRuntimeMeshLoader::LoadMeshFromFile(const FString& FilePath,
                                                FHeliosMeshData& OutMeshData) {
  const FString Extension = FPaths::GetExtension(FilePath).ToLower();
  if (Extension == TEXT("obj")) {
    return LoadObj(FilePath, OutMeshData);
  }
  if (Extension == TEXT("stl")) {
    return LoadStl(FilePath, OutMeshData);
  }

  UE_LOG(LogHeliosMesh, Error,
         TEXT("Helios: unsupported mesh extension \"%s\" for \"%s\""),
         *Extension, *FilePath);
  return false;
}

bool FHeliosRuntimeMeshLoader::LoadObj(const FString& FilePath,
                                       FHeliosMeshData& OutMeshData) {
  TArray<FString> Lines;
  if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath)) {
    UE_LOG(LogHeliosMesh, Error, TEXT("Helios: failed to read OBJ \"%s\""),
           *FilePath);
    return false;
  }

  TArray<FVector> RawVertices;
  TArray<FVector2D> RawUVs;
  TArray<FVector> RawNormals;

  OutMeshData = FHeliosMeshData();

  for (const FString& Line : Lines) {
    const FString Trimmed = Line.TrimStartAndEnd();
    if (Trimmed.StartsWith(TEXT("v "))) {
      TArray<FString> Tokens;
      Trimmed.ParseIntoArrayWS(Tokens);
      if (Tokens.Num() >= 4) {
        // OBJ is right-handed Y-up by convention in many tools, but we treat
        // source axes as direct Z-up meters per the loader's documented
        // assumption; only scale is applied.
        RawVertices.Add(FVector(FCString::Atof(*Tokens[1]),
                                FCString::Atof(*Tokens[2]),
                                FCString::Atof(*Tokens[3])) *
                        MetersToUnreal);
      }
    } else if (Trimmed.StartsWith(TEXT("vt "))) {
      TArray<FString> Tokens;
      Trimmed.ParseIntoArrayWS(Tokens);
      if (Tokens.Num() >= 3) {
        RawUVs.Add(FVector2D(FCString::Atof(*Tokens[1]),
                             1.f - FCString::Atof(*Tokens[2])));
      }
    } else if (Trimmed.StartsWith(TEXT("vn "))) {
      TArray<FString> Tokens;
      Trimmed.ParseIntoArrayWS(Tokens);
      if (Tokens.Num() >= 4) {
        RawNormals.Add(FVector(FCString::Atof(*Tokens[1]),
                               FCString::Atof(*Tokens[2]),
                               FCString::Atof(*Tokens[3])));
      }
    } else if (Trimmed.StartsWith(TEXT("f "))) {
      TArray<FString> Tokens;
      Trimmed.ParseIntoArrayWS(Tokens);
      Tokens.RemoveAt(0);
      if (Tokens.Num() < 3) {
        continue;
      }

      // Parse each "v/vt/vn" group (indices are 1-based; only positive indices
      // are supported).
      TArray<int32> FaceVertexIdx, FaceUvIdx, FaceNormalIdx;
      for (const FString& Token : Tokens) {
        TArray<FString> Parts;
        Token.ParseIntoArray(Parts, TEXT("/"), false);
        FaceVertexIdx.Add(Parts.Num() > 0 && !Parts[0].IsEmpty()
                              ? FCString::Atoi(*Parts[0]) - 1
                              : -1);
        FaceUvIdx.Add(Parts.Num() > 1 && !Parts[1].IsEmpty()
                          ? FCString::Atoi(*Parts[1]) - 1
                          : -1);
        FaceNormalIdx.Add(Parts.Num() > 2 && !Parts[2].IsEmpty()
                              ? FCString::Atoi(*Parts[2]) - 1
                              : -1);
      }

      // Fan-triangulate polygons with more than 3 vertices; vertices are
      // flat-expanded (no welding).
      for (int32 TriIndex = 1; TriIndex < FaceVertexIdx.Num() - 1; ++TriIndex) {
        const int32 Corners[3] = {0, TriIndex, TriIndex + 1};
        for (int32 Corner : Corners) {
          const int32 VIdx = FaceVertexIdx[Corner];
          if (!RawVertices.IsValidIndex(VIdx)) {
            continue;
          }
          OutMeshData.Triangles.Add(OutMeshData.Vertices.Num());
          OutMeshData.Vertices.Add(RawVertices[VIdx]);

          const int32 NIdx = FaceNormalIdx[Corner];
          OutMeshData.Normals.Add(RawNormals.IsValidIndex(NIdx)
                                      ? RawNormals[NIdx]
                                      : FVector::UpVector);

          const int32 UvIdx = FaceUvIdx[Corner];
          OutMeshData.UVs.Add(RawUVs.IsValidIndex(UvIdx)
                                  ? RawUVs[UvIdx]
                                  : FVector2D::ZeroVector);
        }
      }
    }
  }

  if (OutMeshData.IsEmpty()) {
    UE_LOG(LogHeliosMesh, Error,
           TEXT("Helios: OBJ \"%s\" produced no triangles"), *FilePath);
    return false;
  }

  UE_LOG(LogHeliosMesh, Log,
         TEXT("Helios: loaded OBJ \"%s\" (%d verts, %d tris)"), *FilePath,
         OutMeshData.Vertices.Num(), OutMeshData.Triangles.Num() / 3);
  return true;
}

bool FHeliosRuntimeMeshLoader::LoadStl(const FString& FilePath,
                                       FHeliosMeshData& OutMeshData) {
  TArray<uint8> Bytes;
  if (!FFileHelper::LoadFileToArray(Bytes, *FilePath)) {
    UE_LOG(LogHeliosMesh, Error, TEXT("Helios: failed to read STL \"%s\""),
           *FilePath);
    return false;
  }

  // Binary STL also commonly starts with "solid", so a text-prefix check is
  // unreliable; instead check whether the file size matches what the binary
  // triangle-count header predicts (80-byte header + 4-byte count + 50
  // bytes/triangle).
  bool bIsBinary = false;
  if (Bytes.Num() >= 84) {
    const uint32 TriangleCount =
        *reinterpret_cast<const uint32*>(Bytes.GetData() + 80);
    const int64 ExpectedSize = 84 + static_cast<int64>(TriangleCount) * 50;
    bIsBinary = (ExpectedSize == Bytes.Num());
  }

  if (bIsBinary) {
    return LoadStlBinary(Bytes, OutMeshData);
  }

  TArray<FString> Lines;
  if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath)) {
    return false;
  }
  return LoadStlAscii(Lines, OutMeshData);
}

bool FHeliosRuntimeMeshLoader::LoadStlAscii(const TArray<FString>& Lines,
                                            FHeliosMeshData& OutMeshData) {
  OutMeshData = FHeliosMeshData();
  FVector PendingNormal = FVector::UpVector;

  for (const FString& Line : Lines) {
    const FString Trimmed = Line.TrimStartAndEnd();
    if (Trimmed.StartsWith(TEXT("facet normal"))) {
      TArray<FString> Tokens;
      Trimmed.ParseIntoArrayWS(Tokens);
      if (Tokens.Num() >= 5) {
        PendingNormal =
            FVector(FCString::Atof(*Tokens[2]), FCString::Atof(*Tokens[3]),
                    FCString::Atof(*Tokens[4]));
      }
    } else if (Trimmed.StartsWith(TEXT("vertex"))) {
      TArray<FString> Tokens;
      Trimmed.ParseIntoArrayWS(Tokens);
      if (Tokens.Num() >= 4) {
        OutMeshData.Triangles.Add(OutMeshData.Vertices.Num());
        OutMeshData.Vertices.Add(FVector(FCString::Atof(*Tokens[1]),
                                         FCString::Atof(*Tokens[2]),
                                         FCString::Atof(*Tokens[3])) *
                                 MetersToUnreal);
        OutMeshData.Normals.Add(PendingNormal);
        OutMeshData.UVs.Add(FVector2D::ZeroVector);
      }
    }
  }

  if (OutMeshData.IsEmpty()) {
    UE_LOG(LogHeliosMesh, Error,
           TEXT("Helios: ASCII STL produced no triangles"));
    return false;
  }
  return true;
}

bool FHeliosRuntimeMeshLoader::LoadStlBinary(const TArray<uint8>& Bytes,
                                             FHeliosMeshData& OutMeshData) {
  OutMeshData = FHeliosMeshData();

  const uint32 TriangleCount =
      *reinterpret_cast<const uint32*>(Bytes.GetData() + 80);
  const uint8* Cursor = Bytes.GetData() + 84;

  for (uint32 TriIndex = 0; TriIndex < TriangleCount; ++TriIndex) {
    // Each record: 3 floats normal, 3x3 floats vertices, 2 bytes attribute
    // (unused) = 50 bytes.
    const float* Floats = reinterpret_cast<const float*>(Cursor);
    const FVector Normal(Floats[0], Floats[1], Floats[2]);

    for (int32 Corner = 0; Corner < 3; ++Corner) {
      const float* V = Floats + 3 + Corner * 3;
      OutMeshData.Triangles.Add(OutMeshData.Vertices.Num());
      OutMeshData.Vertices.Add(FVector(V[0], V[1], V[2]) * MetersToUnreal);
      OutMeshData.Normals.Add(Normal);
      OutMeshData.UVs.Add(FVector2D::ZeroVector);
    }

    Cursor += 50;
  }

  if (OutMeshData.IsEmpty()) {
    UE_LOG(LogHeliosMesh, Error,
           TEXT("Helios: binary STL produced no triangles"));
    return false;
  }
  return true;
}

void FHeliosRuntimeMeshLoader::ApplyToComponent(
    UProceduralMeshComponent* Component, const FHeliosMeshData& MeshData,
    int32 SectionIndex, bool bCreateCollision) {
  if (!Component || MeshData.IsEmpty()) {
    return;
  }

  TArray<FProcMeshTangent> Tangents = MeshData.Tangents;
  TArray<FVector> Normals = MeshData.Normals;
  if (Tangents.IsEmpty()) {
    // Generate tangents when the source format didn't provide them (OBJ/STL
    // both lack tangent data).
    UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
        MeshData.Vertices, MeshData.Triangles, MeshData.UVs, Normals, Tangents);
  }

  Component->CreateMeshSection_LinearColor(
      SectionIndex, MeshData.Vertices, MeshData.Triangles, Normals,
      MeshData.UVs, TArray<FLinearColor>(), Tangents, bCreateCollision);
}
