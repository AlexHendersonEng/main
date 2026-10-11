// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/HeliosPlaybackWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Slider.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/World.h"
#include "Sequencing/HeliosPlaybackSubsystem.h"
#include "Vehicles/HeliosVehicleActor.h"

namespace {
constexpr float MinPlaybackRate = 0.1f;
constexpr float MaxPlaybackRate = 4.0f;
}  // namespace

void UHeliosPlaybackWidget::NativeOnInitialized() {
  Super::NativeOnInitialized();
  BuildWidgetTree();
}

void UHeliosPlaybackWidget::NativeConstruct() {
  Super::NativeConstruct();

  if (UHeliosPlaybackSubsystem* Playback =
          GetWorld()->GetSubsystem<UHeliosPlaybackSubsystem>()) {
    for (AHeliosVehicleActor* Vehicle : Playback->GetSpawnedVehicles()) {
      if (!Vehicle) {
        continue;
      }

      UCheckBox* CheckBox =
          VehicleVisibilityControls.FindRef(Vehicle->VehicleId);
      if (CheckBox) {
        CheckBox->SetIsChecked(!Vehicle->IsHidden());
      }
    }

    const float NormalizedRate = (Playback->GetPlayRate() - MinPlaybackRate) /
                                 (MaxPlaybackRate - MinPlaybackRate);
    bUpdatingSpeedSlider = true;
    SpeedSlider->SetValue(FMath::Clamp(NormalizedRate, 0.0f, 1.0f));
    bUpdatingSpeedSlider = false;
  }

  RefreshPlaybackControls();
}

void UHeliosPlaybackWidget::NativeTick(const FGeometry& MyGeometry,
                                       float InDeltaTime) {
  Super::NativeTick(MyGeometry, InDeltaTime);
  RefreshPlaybackControls();
}

void UHeliosPlaybackWidget::BuildWidgetTree() {
  UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
      UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
  WidgetTree->RootWidget = Canvas;

  UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),
                                                        TEXT("PlaybackPanel"));
  Panel->SetBrushColor(FLinearColor(0.015f, 0.02f, 0.03f, 0.88f));
  Panel->SetPadding(FMargin(12.0f));
  UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Panel);
  PanelSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
  PanelSlot->SetAlignment(FVector2D(0.0f, 1.0f));
  PanelSlot->SetPosition(FVector2D(24.0f, -24.0f));
  PanelSlot->SetAutoSize(true);

  USizeBox* ContentSize = WidgetTree->ConstructWidget<USizeBox>(
      USizeBox::StaticClass(), TEXT("PlaybackContentSize"));
  ContentSize->SetWidthOverride(360.0f);
  Panel->SetContent(ContentSize);
  UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(
      UVerticalBox::StaticClass(), TEXT("PlaybackLayout"));
  ContentSize->SetContent(Layout);

  UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(
      UTextBlock::StaticClass(), TEXT("Title"));
  Title->SetText(FText::FromString(TEXT("Helios Playback")));
  Layout->AddChildToVerticalBox(Title);

  UHorizontalBox* TransportRow = WidgetTree->ConstructWidget<UHorizontalBox>(
      UHorizontalBox::StaticClass(), TEXT("TransportRow"));
  Layout->AddChildToVerticalBox(TransportRow);

  PlayPauseButton = WidgetTree->ConstructWidget<UButton>(
      UButton::StaticClass(), TEXT("PlayPauseButton"));
  PlayPauseButton->OnClicked.AddDynamic(this,
                                        &UHeliosPlaybackWidget::TogglePlayback);
  PlayPauseLabel = WidgetTree->ConstructWidget<UTextBlock>(
      UTextBlock::StaticClass(), TEXT("PlayPauseLabel"));
  PlayPauseButton->SetContent(PlayPauseLabel);
  TransportRow->AddChildToHorizontalBox(PlayPauseButton);

  TimeLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
                                                      TEXT("TimeLabel"));
  TransportRow->AddChildToHorizontalBox(TimeLabel);

  TimeSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(),
                                                    TEXT("TimeSlider"));
  TimeSlider->OnValueChanged.AddDynamic(
      this, &UHeliosPlaybackWidget::OnTimeSliderChanged);
  TimeSlider->OnMouseCaptureBegin.AddDynamic(
      this, &UHeliosPlaybackWidget::OnTimeCaptureBegin);
  TimeSlider->OnMouseCaptureEnd.AddDynamic(
      this, &UHeliosPlaybackWidget::OnTimeCaptureEnd);
  TimeSlider->OnControllerCaptureBegin.AddDynamic(
      this, &UHeliosPlaybackWidget::OnTimeCaptureBegin);
  TimeSlider->OnControllerCaptureEnd.AddDynamic(
      this, &UHeliosPlaybackWidget::OnTimeCaptureEnd);
  Layout->AddChildToVerticalBox(TimeSlider);

  UHorizontalBox* SpeedRow = WidgetTree->ConstructWidget<UHorizontalBox>(
      UHorizontalBox::StaticClass(), TEXT("SpeedRow"));
  Layout->AddChildToVerticalBox(SpeedRow);

  SpeedLabel = WidgetTree->ConstructWidget<UTextBlock>(
      UTextBlock::StaticClass(), TEXT("SpeedLabel"));
  SpeedRow->AddChildToHorizontalBox(SpeedLabel);

  SpeedSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(),
                                                     TEXT("SpeedSlider"));
  SpeedSlider->OnValueChanged.AddDynamic(
      this, &UHeliosPlaybackWidget::OnSpeedSliderChanged);
  UHorizontalBoxSlot* SpeedSliderSlot =
      SpeedRow->AddChildToHorizontalBox(SpeedSlider);
  SpeedSliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

  UTextBlock* VisibilityTitle = WidgetTree->ConstructWidget<UTextBlock>(
      UTextBlock::StaticClass(), TEXT("VisibilityTitle"));
  VisibilityTitle->SetText(FText::FromString(TEXT("Vehicles")));
  Layout->AddChildToVerticalBox(VisibilityTitle);

  if (UHeliosPlaybackSubsystem* Playback =
          GetWorld()->GetSubsystem<UHeliosPlaybackSubsystem>()) {
    for (AHeliosVehicleActor* Vehicle : Playback->GetSpawnedVehicles()) {
      if (!Vehicle) {
        continue;
      }

      UHorizontalBox* VehicleRow = WidgetTree->ConstructWidget<UHorizontalBox>(
          UHorizontalBox::StaticClass());
      Layout->AddChildToVerticalBox(VehicleRow);

      UCheckBox* CheckBox =
          WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
      CheckBox->SetIsChecked(!Vehicle->IsHidden());
      CheckBox->OnCheckStateChanged.AddDynamic(
          this, &UHeliosPlaybackWidget::OnVehicleVisibilityChanged);
      VehicleRow->AddChildToHorizontalBox(CheckBox);
      VehicleVisibilityControls.Add(Vehicle->VehicleId, CheckBox);

      UTextBlock* VehicleLabel =
          WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
      VehicleLabel->SetText(FText::FromString(Vehicle->VehicleId));
      VehicleRow->AddChildToHorizontalBox(VehicleLabel);
    }
  }

  // Rendering is user-triggered so large exports do not start during preview
  // setup.
  UButton* RenderButton = WidgetTree->ConstructWidget<UButton>(
      UButton::StaticClass(), TEXT("RenderExportsButton"));
  UTextBlock* RenderLabel = WidgetTree->ConstructWidget<UTextBlock>(
      UTextBlock::StaticClass(), TEXT("RenderExportsLabel"));
  RenderLabel->SetText(FText::FromString(TEXT("Render configured cameras")));
  RenderButton->SetContent(RenderLabel);
  RenderButton->OnClicked.AddDynamic(
      this, &UHeliosPlaybackWidget::OnRenderExportsClicked);
  Layout->AddChildToVerticalBox(RenderButton);
}

void UHeliosPlaybackWidget::TogglePlayback() {
  if (UHeliosPlaybackSubsystem* Playback =
          GetWorld()->GetSubsystem<UHeliosPlaybackSubsystem>()) {
    if (Playback->IsPlaying()) {
      Playback->Pause();
    } else {
      Playback->Play();
    }
  }
  RefreshPlaybackControls();
}

void UHeliosPlaybackWidget::OnTimeSliderChanged(float Value) {
  if (bIsDraggingTime) {
    if (UHeliosPlaybackSubsystem* Playback =
            GetWorld()->GetSubsystem<UHeliosPlaybackSubsystem>()) {
      Playback->SetPlaybackPositionSeconds(Value *
                                           Playback->GetDurationSeconds());
    }
  }
}

void UHeliosPlaybackWidget::OnTimeCaptureBegin() {
  bIsDraggingTime = true;
  if (UHeliosPlaybackSubsystem* Playback =
          GetWorld()->GetSubsystem<UHeliosPlaybackSubsystem>()) {
    // Pause while scrubbing, then restore playback only if it was already
    // running.
    bWasPlayingBeforeScrub = Playback->IsPlaying();
    Playback->Pause();
  }
}

void UHeliosPlaybackWidget::OnTimeCaptureEnd() {
  bIsDraggingTime = false;
  if (bWasPlayingBeforeScrub) {
    if (UHeliosPlaybackSubsystem* Playback =
            GetWorld()->GetSubsystem<UHeliosPlaybackSubsystem>()) {
      Playback->Play();
    }
  }
  bWasPlayingBeforeScrub = false;
}

void UHeliosPlaybackWidget::OnSpeedSliderChanged(float Value) {
  if (bUpdatingSpeedSlider) {
    return;
  }

  const float Rate = FMath::Lerp(MinPlaybackRate, MaxPlaybackRate, Value);
  if (UHeliosPlaybackSubsystem* Playback =
          GetWorld()->GetSubsystem<UHeliosPlaybackSubsystem>()) {
    Playback->SetPlayRate(Rate);
  }
  SpeedLabel->SetText(
      FText::FromString(FString::Printf(TEXT("Speed: %.1fx"), Rate)));
}

void UHeliosPlaybackWidget::OnVehicleVisibilityChanged(bool bIsVisible) {
  if (UHeliosPlaybackSubsystem* Playback =
          GetWorld()->GetSubsystem<UHeliosPlaybackSubsystem>()) {
    for (const TPair<FString, TObjectPtr<UCheckBox>>& Entry :
         VehicleVisibilityControls) {
      Playback->SetVehicleVisible(Entry.Key, Entry.Value->IsChecked());
    }
  }
}

void UHeliosPlaybackWidget::OnRenderExportsClicked() {
  if (UHeliosPlaybackSubsystem* Playback =
          GetWorld()->GetSubsystem<UHeliosPlaybackSubsystem>()) {
    Playback->RenderConfiguredCameras();
  }
}

void UHeliosPlaybackWidget::RefreshPlaybackControls() {
  UHeliosPlaybackSubsystem* Playback =
      GetWorld()->GetSubsystem<UHeliosPlaybackSubsystem>();
  if (!Playback) {
    return;
  }

  const float Duration = Playback->GetDurationSeconds();
  const float CurrentTime = Playback->GetCurrentTimeSeconds();
  TimeLabel->SetText(FText::FromString(
      FString::Printf(TEXT("%.2f / %.2f s"), CurrentTime, Duration)));
  if (!bIsDraggingTime) {
    TimeSlider->SetValue(Duration > 0.0f
                             ? FMath::Clamp(CurrentTime / Duration, 0.0f, 1.0f)
                             : 0.0f);
  }

  PlayPauseLabel->SetText(
      FText::FromString(Playback->IsPlaying() ? TEXT("Pause") : TEXT("Play")));
  const float Rate = Playback->GetPlayRate();
  SpeedLabel->SetText(
      FText::FromString(FString::Printf(TEXT("Speed: %.1fx"), Rate)));
}
