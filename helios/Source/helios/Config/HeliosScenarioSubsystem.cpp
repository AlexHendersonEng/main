// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeliosScenarioSubsystem.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeliosConfig, Log, All);

// Auto-loads the default/overridden scenario file on GameInstance startup.
void UHeliosScenarioSubsystem::Initialize(
    FSubsystemCollectionBase& Collection) {
  Super::Initialize(Collection);

  const FString ConfigPath = ResolveConfigFilePath();
  if (!LoadScenarioFromFile(ConfigPath)) {
    UE_LOG(LogHeliosConfig, Warning,
           TEXT("Helios: no scenario loaded on startup (looked for \"%s\"). "
                "Call LoadScenarioFromFile to load one manually."),
           *ConfigPath);
  }
}

// "-HeliosConfig=<path>" override, else the bundled sample scenario.
FString UHeliosScenarioSubsystem::ResolveConfigFilePath() const {
  FString OverridePath;
  if (FParse::Value(FCommandLine::Get(), TEXT("HeliosConfig="), OverridePath) &&
      !OverridePath.IsEmpty()) {
    return OverridePath;
  }

  return FPaths::Combine(FPaths::ProjectDir(),
                         TEXT("Config/Scenarios/sample_scenario.json"));
}

bool UHeliosScenarioSubsystem::LoadScenarioFromFile(const FString& FilePath) {
  // Read the file to a string.
  FString JsonString;
  if (!FFileHelper::LoadFileToString(JsonString, *FilePath)) {
    UE_LOG(LogHeliosConfig, Error,
           TEXT("Helios: failed to read scenario config file \"%s\""),
           *FilePath);
    bScenarioLoaded = false;
    return false;
  }

  // Parse into a generic JSON DOM (not straight into the struct) so each field
  // can be validated individually and bad entries logged/skipped rather than
  // failing the whole file.
  TSharedPtr<FJsonObject> RootObject;
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(JsonString);
  if (!FJsonSerializer::Deserialize(Reader, RootObject) ||
      !RootObject.IsValid()) {
    UE_LOG(LogHeliosConfig, Error,
           TEXT("Helios: failed to parse JSON in scenario config file \"%s\""),
           *FilePath);
    bScenarioLoaded = false;
    return false;
  }

  // Parse into a temporary so a bad file never partially overwrites the active
  // scenario.
  FHeliosScenarioConfig NewScenario;
  if (!ParseScenarioJson(RootObject, NewScenario)) {
    UE_LOG(LogHeliosConfig, Error,
           TEXT("Helios: scenario config file \"%s\" was invalid"), *FilePath);
    bScenarioLoaded = false;
    return false;
  }

  Scenario = MoveTemp(NewScenario);
  bScenarioLoaded = true;
  LogScenarioSummary();
  return true;
}

bool UHeliosScenarioSubsystem::ParseScenarioJson(
    const TSharedPtr<FJsonObject>& RootObject,
    FHeliosScenarioConfig& OutScenario) const {
  RootObject->TryGetStringField(TEXT("scenario_name"),
                                OutScenario.ScenarioName);

  // Each vehicle/camera entry is parsed independently; a bad entry is skipped
  // (with a warning) rather than failing the whole file.
  const TArray<TSharedPtr<FJsonValue>>* VehiclesArray = nullptr;
  if (RootObject->TryGetArrayField(TEXT("vehicles"), VehiclesArray)) {
    for (const TSharedPtr<FJsonValue>& VehicleValue : *VehiclesArray) {
      FHeliosVehicleConfig Vehicle;
      if (VehicleValue.IsValid() &&
          ParseVehicle(VehicleValue->AsObject(), Vehicle)) {
        OutScenario.Vehicles.Add(MoveTemp(Vehicle));
      } else {
        UE_LOG(LogHeliosConfig, Warning,
               TEXT("Helios: skipped an invalid vehicle entry"));
      }
    }
  }

  const TArray<TSharedPtr<FJsonValue>>* CamerasArray = nullptr;
  if (RootObject->TryGetArrayField(TEXT("cameras"), CamerasArray)) {
    for (const TSharedPtr<FJsonValue>& CameraValue : *CamerasArray) {
      FHeliosCameraConfig Camera;
      if (CameraValue.IsValid() &&
          ParseCamera(CameraValue->AsObject(), Camera)) {
        OutScenario.Cameras.Add(MoveTemp(Camera));
      } else {
        UE_LOG(LogHeliosConfig, Warning,
               TEXT("Helios: skipped an invalid camera entry"));
      }
    }
  }

  const TSharedPtr<FJsonObject>* PlaybackObject = nullptr;
  if (RootObject->TryGetObjectField(TEXT("playback"), PlaybackObject)) {
    ParsePlayback(*PlaybackObject, OutScenario.Playback);
  }

  return true;
}

// Only "id" is required; everything else defaults if omitted.
bool UHeliosScenarioSubsystem::ParseVehicle(
    const TSharedPtr<FJsonObject>& VehicleObject,
    FHeliosVehicleConfig& OutVehicle) const {
  if (!VehicleObject.IsValid()) {
    return false;
  }

  if (!VehicleObject->TryGetStringField(TEXT("id"), OutVehicle.Id) ||
      OutVehicle.Id.IsEmpty()) {
    UE_LOG(LogHeliosConfig, Warning,
           TEXT("Helios: vehicle entry is missing a non-empty \"id\""));
    return false;
  }

  FString TypeString;
  VehicleObject->TryGetStringField(TEXT("type"), TypeString);
  OutVehicle.Type = VehicleTypeFromString(TypeString);

  VehicleObject->TryGetStringField(TEXT("model_path"), OutVehicle.ModelPath);
  VehicleObject->TryGetStringField(TEXT("trajectory_csv"),
                                   OutVehicle.TrajectoryCsvPath);

  // Read as double (Json numbers are always double) then narrow to float.
  double ScaleValue = OutVehicle.Scale;
  if (VehicleObject->TryGetNumberField(TEXT("scale"), ScaleValue)) {
    OutVehicle.Scale = static_cast<float>(ScaleValue);
  }

  VehicleObject->TryGetBoolField(TEXT("visible"), OutVehicle.bVisible);

  return true;
}

// Only "id" is required; empty/omitted "attached_to" means world-fixed.
bool UHeliosScenarioSubsystem::ParseCamera(
    const TSharedPtr<FJsonObject>& CameraObject,
    FHeliosCameraConfig& OutCamera) const {
  if (!CameraObject.IsValid()) {
    return false;
  }

  if (!CameraObject->TryGetStringField(TEXT("id"), OutCamera.Id) ||
      OutCamera.Id.IsEmpty()) {
    UE_LOG(LogHeliosConfig, Warning,
           TEXT("Helios: camera entry is missing a non-empty \"id\""));
    return false;
  }

  CameraObject->TryGetStringField(TEXT("attached_to"),
                                  OutCamera.AttachedToVehicleId);

  const TSharedPtr<FJsonObject>* LocationObject = nullptr;
  if (CameraObject->TryGetObjectField(TEXT("location"), LocationObject)) {
    double X = 0.0, Y = 0.0, Z = 0.0;
    (*LocationObject)->TryGetNumberField(TEXT("x"), X);
    (*LocationObject)->TryGetNumberField(TEXT("y"), Y);
    (*LocationObject)->TryGetNumberField(TEXT("z"), Z);
    OutCamera.Location = FVector(X, Y, Z);
  }

  const TSharedPtr<FJsonObject>* RotationObject = nullptr;
  if (CameraObject->TryGetObjectField(TEXT("rotation"), RotationObject)) {
    double Pitch = 0.0, Yaw = 0.0, Roll = 0.0;
    (*RotationObject)->TryGetNumberField(TEXT("pitch"), Pitch);
    (*RotationObject)->TryGetNumberField(TEXT("yaw"), Yaw);
    (*RotationObject)->TryGetNumberField(TEXT("roll"), Roll);
    OutCamera.Rotation = FRotator(Pitch, Yaw, Roll);
  }

  double FovValue = OutCamera.FieldOfView;
  if (CameraObject->TryGetNumberField(TEXT("fov"), FovValue)) {
    OutCamera.FieldOfView = static_cast<float>(FovValue);
  }

  const TSharedPtr<FJsonObject>* ExportObject = nullptr;
  if (CameraObject->TryGetObjectField(TEXT("export"), ExportObject)) {
    ParseExport(*ExportObject, OutCamera.Export);
  }

  return true;
}

void UHeliosScenarioSubsystem::ParseExport(
    const TSharedPtr<FJsonObject>& ExportObject,
    FHeliosCameraExportConfig& OutExport) const {
  if (!ExportObject.IsValid()) {
    return;
  }

  ExportObject->TryGetBoolField(TEXT("enabled"), OutExport.bEnabled);

  // Unrecognised mode strings resolve to None so a typo can't trigger an
  // unintended render.
  FString ModeString;
  ExportObject->TryGetStringField(TEXT("mode"), ModeString);
  OutExport.Mode = ExportModeFromString(ModeString);

  int32 ResX = OutExport.ResolutionX;
  int32 ResY = OutExport.ResolutionY;
  ExportObject->TryGetNumberField(TEXT("resolution_x"), ResX);
  ExportObject->TryGetNumberField(TEXT("resolution_y"), ResY);
  OutExport.ResolutionX = ResX;
  OutExport.ResolutionY = ResY;

  double FrameRateValue = OutExport.FrameRate;
  if (ExportObject->TryGetNumberField(TEXT("frame_rate"), FrameRateValue)) {
    OutExport.FrameRate = static_cast<float>(FrameRateValue);
  }

  ExportObject->TryGetStringField(TEXT("output_directory"),
                                  OutExport.OutputDirectory);
  ExportObject->TryGetStringField(TEXT("output_name"), OutExport.OutputName);
}

void UHeliosScenarioSubsystem::ParsePlayback(
    const TSharedPtr<FJsonObject>& PlaybackObject,
    FHeliosPlaybackConfig& OutPlayback) const {
  if (!PlaybackObject.IsValid()) {
    return;
  }

  double SpeedValue = OutPlayback.DefaultSpeed;
  if (PlaybackObject->TryGetNumberField(TEXT("default_speed"), SpeedValue)) {
    OutPlayback.DefaultSpeed = static_cast<float>(SpeedValue);
  }

  PlaybackObject->TryGetBoolField(TEXT("loop"), OutPlayback.bLoop);
}

// Unrecognised/missing strings default to Aircraft.
EHeliosVehicleType UHeliosScenarioSubsystem::VehicleTypeFromString(
    const FString& InString) {
  if (InString.Equals(TEXT("GroundVehicle"), ESearchCase::IgnoreCase)) {
    return EHeliosVehicleType::GroundVehicle;
  }
  if (InString.Equals(TEXT("Boat"), ESearchCase::IgnoreCase)) {
    return EHeliosVehicleType::Boat;
  }
  if (InString.Equals(TEXT("UnderwaterVessel"), ESearchCase::IgnoreCase)) {
    return EHeliosVehicleType::UnderwaterVessel;
  }
  return EHeliosVehicleType::Aircraft;
}

// Unrecognised/missing strings default to None (export disabled).
EHeliosCameraExportMode UHeliosScenarioSubsystem::ExportModeFromString(
    const FString& InString) {
  if (InString.Equals(TEXT("Video"), ESearchCase::IgnoreCase)) {
    return EHeliosCameraExportMode::Video;
  }
  if (InString.Equals(TEXT("ImageSequence"), ESearchCase::IgnoreCase)) {
    return EHeliosCameraExportMode::ImageSequence;
  }
  return EHeliosCameraExportMode::None;
}

// Diagnostic log so a loaded scenario can be confirmed from the Output Log.
void UHeliosScenarioSubsystem::LogScenarioSummary() const {
  UE_LOG(
      LogHeliosConfig, Log,
      TEXT(
          "Helios: loaded scenario \"%s\" with %d vehicle(s) and %d camera(s)"),
      *Scenario.ScenarioName, Scenario.Vehicles.Num(), Scenario.Cameras.Num());

  for (const FHeliosVehicleConfig& Vehicle : Scenario.Vehicles) {
    UE_LOG(
        LogHeliosConfig, Log,
        TEXT("Helios:   vehicle \"%s\" type=%d model=\"%s\" trajectory=\"%s\""),
        *Vehicle.Id, static_cast<int32>(Vehicle.Type), *Vehicle.ModelPath,
        *Vehicle.TrajectoryCsvPath);
  }

  for (const FHeliosCameraConfig& Camera : Scenario.Cameras) {
    UE_LOG(LogHeliosConfig, Log,
           TEXT("Helios:   camera \"%s\" attachedTo=\"%s\" exportEnabled=%s"),
           *Camera.Id, *Camera.AttachedToVehicleId,
           Camera.Export.bEnabled ? TEXT("true") : TEXT("false"));
  }
}
