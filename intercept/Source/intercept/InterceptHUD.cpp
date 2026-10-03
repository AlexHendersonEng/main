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
