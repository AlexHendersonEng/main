// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeliosScenarioTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HeliosScenarioSubsystem.generated.h"

class FJsonObject;

/**
 * Loads and holds the active Helios scenario configuration (vehicles, cameras,
 * playback settings) parsed from a JSON file on disk. Defaults to
 * "<ProjectDir>/Config/Scenarios/sample_scenario.json", overridable with the
 * "-HeliosConfig=<path>" command line switch.
 */
UCLASS()
class UHeliosScenarioSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()

 public:
  // Auto-loads the scenario file when the owning GameInstance starts up.
  virtual void Initialize(FSubsystemCollectionBase& Collection) override;

  /** Loads a scenario JSON file from disk, replacing any currently loaded
   * scenario. Returns true on success. */
  UFUNCTION(BlueprintCallable, Category = "Helios|Config")
  bool LoadScenarioFromFile(const FString& FilePath);

  /** True once a scenario has been successfully parsed. */
  UFUNCTION(BlueprintCallable, Category = "Helios|Config")
  bool HasLoadedScenario() const { return bScenarioLoaded; }

  /** Read-only access to the currently loaded scenario. */
  UFUNCTION(BlueprintCallable, Category = "Helios|Config")
  const FHeliosScenarioConfig& GetScenario() const { return Scenario; }

 private:
  /** Resolves the config file path: command-line override, else the default
   * sample scenario path. */
  FString ResolveConfigFilePath() const;

  // JSON -> struct parsing helpers, split per-section so one bad entry doesn't
  // fail the whole file.
  bool ParseScenarioJson(const TSharedPtr<FJsonObject>& RootObject,
                         FHeliosScenarioConfig& OutScenario) const;
  bool ParseVehicle(const TSharedPtr<FJsonObject>& VehicleObject,
                    FHeliosVehicleConfig& OutVehicle) const;
  bool ParseCamera(const TSharedPtr<FJsonObject>& CameraObject,
                   FHeliosCameraConfig& OutCamera) const;
  void ParseExport(const TSharedPtr<FJsonObject>& ExportObject,
                   FHeliosCameraExportConfig& OutExport) const;
  void ParsePlayback(const TSharedPtr<FJsonObject>& PlaybackObject,
                     FHeliosPlaybackConfig& OutPlayback) const;

  /** Defaults to Aircraft if unrecognised. */
  static EHeliosVehicleType VehicleTypeFromString(const FString& InString);
  /** Defaults to None (export disabled) if unrecognised. */
  static EHeliosCameraExportMode ExportModeFromString(const FString& InString);

  /** Logs a short summary of the loaded scenario for verification. */
  void LogScenarioSummary() const;

  UPROPERTY()
  FHeliosScenarioConfig Scenario;

  bool bScenarioLoaded = false;
};
