#include "InterceptHUD.h"

#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "InterceptGameMode.h"
#include "InterceptLauncherComponent.h"
#include "InterceptLockOnComponent.h"
#include "InterceptPlayerPawn.h"
#include "InterceptTarget.h"

void AInterceptHUD::DrawHUD() {
  Super::DrawHUD();
  if (!Canvas) {
    return;
  }
  DrawReticle();
  DrawTargetMarkers();
  DrawReadouts();
  DrawHelp();
}

void AInterceptHUD::DrawHelp() {
  const float Margin = 20.f;
  const float LineHeight = 24.f * TextScale;

  if (!bShowHelp) {
    const FString Hint = TEXT("H - Help");
    float W = 0.f, H = 0.f;
    GetTextSize(Hint, W, H, nullptr, TextScale);
    DrawText(Hint, NormalColor, Canvas->ClipX - W - Margin, Margin, nullptr,
             TextScale);
    return;
  }

  static const TCHAR* Lines[] = {
      TEXT("HOW TO PLAY"),
      TEXT(""),
      TEXT("Stop the incoming targets before they reach the base."),
      TEXT("Lock onto a target, then launch a guided interceptor at it."),
      TEXT(""),
      TEXT("W A S D  -  Move"),
      TEXT("Mouse  -  Look / aim"),
      TEXT("Space  -  Jump"),
      TEXT("Right mouse button  -  Lock on / release lock"),
      TEXT("Tab  -  Switch lock to another target"),
      TEXT("Left mouse button  -  Launch interceptor (needs a lock)"),
      TEXT("H  -  Show / hide this help"),
      TEXT(""),
      TEXT("Aim at a target (inside the reticle) to lock it."),
      TEXT("Ammo slowly reloads and is topped up after each wave."),
      TEXT("Each target that gets through costs a life."),
  };

  // Size the panel to the widest line so it fits at any TextScale.
  float PanelW = 0.f;
  for (const TCHAR* Line : Lines) {
    float W = 0.f, H = 0.f;
    GetTextSize(Line, W, H, nullptr, TextScale);
    PanelW = FMath::Max(PanelW, W);
  }
  const float Pad = 24.f;
  const float PanelH = LineHeight * UE_ARRAY_COUNT(Lines);
  const float X = (Canvas->ClipX - PanelW) * 0.5f;
  const float Y = (Canvas->ClipY - PanelH) * 0.5f;

  DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.7f), X - Pad, Y - Pad,
           PanelW + 2.f * Pad, PanelH + 2.f * Pad);

  for (int32 i = 0; i < UE_ARRAY_COUNT(Lines); ++i) {
    DrawText(Lines[i], i == 0 ? LockColor : NormalColor, X, Y + i * LineHeight,
             nullptr, TextScale);
  }
}

void AInterceptHUD::DrawReticle() {
  const float CX = Canvas->ClipX * 0.5f;
  const float CY = Canvas->ClipY * 0.5f;
  constexpr float Gap = 6.f;
  constexpr float Len = 10.f;
  DrawLine(CX - Gap - Len, CY, CX - Gap, CY, NormalColor);
  DrawLine(CX + Gap, CY, CX + Gap + Len, CY, NormalColor);
  DrawLine(CX, CY - Gap - Len, CX, CY - Gap, NormalColor);
  DrawLine(CX, CY + Gap, CX, CY + Gap + Len, NormalColor);
}

void AInterceptHUD::DrawTargetMarkers() {
  const auto* Pawn = Cast<AInterceptPlayerPawn>(GetOwningPawn());
  const AInterceptTarget* Locked = (Pawn && Pawn->GetLockOn())
                                       ? Pawn->GetLockOn()->GetLockedTarget()
                                       : nullptr;

  for (TActorIterator<AInterceptTarget> It(GetWorld()); It; ++It) {
    // Project() returns screen coordinates with Z > 0 only for points in front
    // of the camera, which filters out targets behind the player.
    const FVector Screen = Project(It->GetActorLocation());
    if (Screen.Z <= 0.f) {
      continue;
    }

    if (*It == Locked) {
      constexpr float Half = 22.f;
      DrawLine(Screen.X - Half, Screen.Y - Half, Screen.X + Half,
               Screen.Y - Half, LockColor, 2.f);
      DrawLine(Screen.X + Half, Screen.Y - Half, Screen.X + Half,
               Screen.Y + Half, LockColor, 2.f);
      DrawLine(Screen.X + Half, Screen.Y + Half, Screen.X - Half,
               Screen.Y + Half, LockColor, 2.f);
      DrawLine(Screen.X - Half, Screen.Y + Half, Screen.X - Half,
               Screen.Y - Half, LockColor, 2.f);
    } else {
      constexpr float Half = 10.f;
      DrawLine(Screen.X - Half, Screen.Y, Screen.X, Screen.Y - Half,
               NormalColor);
      DrawLine(Screen.X, Screen.Y - Half, Screen.X + Half, Screen.Y,
               NormalColor);
      DrawLine(Screen.X + Half, Screen.Y, Screen.X, Screen.Y + Half,
               NormalColor);
      DrawLine(Screen.X, Screen.Y + Half, Screen.X - Half, Screen.Y,
               NormalColor);
    }
  }
}

void AInterceptHUD::DrawReadouts() {
  const auto* Mode = GetWorld()->GetAuthGameMode<AInterceptGameMode>();
  const auto* Pawn = Cast<AInterceptPlayerPawn>(GetOwningPawn());
  const float Margin = 20.f;
  const float LineHeight = 24.f * TextScale;
  float Y = Margin;

  if (Mode) {
    DrawText(FString::Printf(TEXT("Wave %d"), Mode->GetWave()), NormalColor,
             Margin, Y, nullptr, TextScale);
    Y += LineHeight;
    DrawText(FString::Printf(TEXT("Score %d"), Mode->GetScore()), NormalColor,
             Margin, Y, nullptr, TextScale);
    Y += LineHeight;
    DrawText(FString::Printf(TEXT("Lives %d"), Mode->GetLives()), NormalColor,
             Margin, Y, nullptr, TextScale);
  }

  if (Pawn && Pawn->GetLauncher()) {
    const UInterceptLauncherComponent* Launcher = Pawn->GetLauncher();
    DrawText(FString::Printf(TEXT("Interceptors %d / %d"), Launcher->GetAmmo(),
                             Launcher->GetMaxAmmo()),
             NormalColor, Margin, Canvas->ClipY - Margin - LineHeight, nullptr,
             TextScale);
  }

  if (Mode && Mode->IsGameOver()) {
    const FString Text = TEXT("GAME OVER");
    float W = 0.f, H = 0.f;
    GetTextSize(Text, W, H, nullptr, TextScale * 3.f);
    DrawText(Text, LockColor, (Canvas->ClipX - W) * 0.5f,
             (Canvas->ClipY - H) * 0.5f, nullptr, TextScale * 3.f);
  }
}
