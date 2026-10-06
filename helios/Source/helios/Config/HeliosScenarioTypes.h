// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeliosScenarioTypes.generated.h"

/** The domain a vehicle belongs to; drives environment and visual behaviour
 * (water, fog, etc). */
UENUM(BlueprintType)
enum class EHeliosVehicleType : uint8 {
  Aircraft,
  GroundVehicle,
  Boat,
  UnderwaterVessel
};

/** How a camera's render output should be produced. */
UENUM(BlueprintType)
enum class EHeliosCameraExportMode : uint8 { None, Video, ImageSequence };

/** Render/export settings for a single camera, parsed from the "export" block
 * of a camera config entry. */
USTRUCT(BlueprintType)
struct FHeliosCameraExportConfig {
  GENERATED_BODY()

  /** Whether this camera should be rendered by the Movie Render Queue
   * integration (Checkpoint 5). */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Export")
  bool bEnabled = false;

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Export")
  EHeliosCameraExportMode Mode = EHeliosCameraExportMode::None;

  /** Output resolution in pixels. */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Export")
  int32 ResolutionX = 1920;

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Export")
  int32 ResolutionY = 1080;

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Export")
  float FrameRate = 30.f;

  /** Absolute or project-relative output directory. */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Export")
  FString OutputDirectory;

  /** Base filename (no extension). */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Export")
  FString OutputName;
};

/** A single camera entry: either world-fixed (AttachedToVehicleId empty) or
 * attached to a named vehicle. */
USTRUCT(BlueprintType)
struct FHeliosCameraConfig {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Camera")
  FString Id;

  /** Vehicle Id to attach to, or empty for a world-fixed camera. */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Camera")
  FString AttachedToVehicleId;

  /** Offset from world origin, or from the attached vehicle's origin. */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Camera")
  FVector Location = FVector::ZeroVector;

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Camera")
  FRotator Rotation = FRotator::ZeroRotator;

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Camera")
  float FieldOfView = 90.f;

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Camera")
  FHeliosCameraExportConfig Export;
};

/** A single vehicle entry: its runtime-loaded mesh and the CSV
 * trajectory/attitude data that drives it. */
USTRUCT(BlueprintType)
struct FHeliosVehicleConfig {
  GENERATED_BODY()

  /** Unique Id, referenced by camera "attached_to" fields. */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Vehicle")
  FString Id;

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Vehicle")
  EHeliosVehicleType Type = EHeliosVehicleType::Aircraft;

  /** Path to a .obj or .stl file, loaded at runtime. */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Vehicle")
  FString ModelPath;

  /** Path to a .csv file with columns: time,x,y,z,roll,pitch,yaw. */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Vehicle")
  FString TrajectoryCsvPath;

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Vehicle")
  float Scale = 1.f;

  /** Initial visibility; toggleable live via the playback UI (Checkpoint 4). */
  UPROPERTY(BlueprintReadOnly, Category = "Helios|Vehicle")
  bool bVisible = true;
};

/** Global playback defaults, controllable at runtime via the UI sliders. */
USTRUCT(BlueprintType)
struct FHeliosPlaybackConfig {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Playback")
  float DefaultSpeed = 1.f;

  UPROPERTY(BlueprintReadOnly, Category = "Helios|Playback")
  bool bLoop = false;
};

/** Root scenario configuration parsed from a single JSON file. */
USTRUCT(BlueprintType)
struct FHeliosScenarioConfig {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Helios")
  FString ScenarioName;

  UPROPERTY(BlueprintReadOnly, Category = "Helios")
  TArray<FHeliosVehicleConfig> Vehicles;

  UPROPERTY(BlueprintReadOnly, Category = "Helios")
  TArray<FHeliosCameraConfig> Cameras;

  UPROPERTY(BlueprintReadOnly, Category = "Helios")
  FHeliosPlaybackConfig Playback;
};
