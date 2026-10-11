// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint\UserWidget.h"
#include "HeliosPlaybackWidget.generated.h"

class UButton;
class UCheckBox;
class UTextBlock;
class USlider;

/** Runtime controls for preview playback and configured camera exports. */
UCLASS()
class HELIOS_API UHeliosPlaybackWidget : public UUserWidget {
  GENERATED_BODY()

 protected:
  virtual void NativeOnInitialized() override;
  virtual void NativeConstruct() override;
  virtual void NativeTick(const FGeometry& MyGeometry,
                          float InDeltaTime) override;

 private:
  UFUNCTION()
  void TogglePlayback();

  UFUNCTION()
  void OnTimeSliderChanged(float Value);

  UFUNCTION()
  void OnTimeCaptureBegin();

  UFUNCTION()
  void OnTimeCaptureEnd();

  UFUNCTION()
  void OnSpeedSliderChanged(float Value);

  UFUNCTION()
  void OnVehicleVisibilityChanged(bool bIsVisible);

  UFUNCTION()
  void OnRenderExportsClicked();

  void BuildWidgetTree();
  void RefreshPlaybackControls();

  UPROPERTY(Transient)
  TObjectPtr<UButton> PlayPauseButton;

  UPROPERTY(Transient)
  TObjectPtr<UTextBlock> PlayPauseLabel;

  UPROPERTY(Transient)
  TObjectPtr<UTextBlock> TimeLabel;

  UPROPERTY(Transient)
  TObjectPtr<USlider> TimeSlider;

  UPROPERTY(Transient)
  TObjectPtr<UTextBlock> SpeedLabel;

  UPROPERTY(Transient)
  TObjectPtr<USlider> SpeedSlider;

  UPROPERTY(Transient)
  TMap<FString, TObjectPtr<UCheckBox>> VehicleVisibilityControls;

  bool bIsDraggingTime = false;
  bool bWasPlayingBeforeScrub = false;
  bool bUpdatingSpeedSlider = false;
};
