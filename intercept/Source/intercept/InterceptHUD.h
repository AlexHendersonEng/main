#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "InterceptHUD.generated.h"

/**
 * Canvas HUD for the interceptor game.
 * Draws a reticle, a lock indicator on the locked target, a marker on every
 * incoming target, and ammo, wave, score and lives readouts. Game state is read
 * each frame from the game mode and the player pawn's components, so the HUD
 * holds no state of its own. A game-over banner is shown when the game ends.
 */
UCLASS()
class INTERCEPT_API AInterceptHUD : public AHUD {
  GENERATED_BODY()

 public:
  virtual void DrawHUD() override;

 protected:
  /** Colour of the reticle and unlocked target markers. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
  FLinearColor NormalColor = FLinearColor::White;

  /** Colour of the lock box on the locked target. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
  FLinearColor LockColor = FLinearColor::Red;

  /** Text scale for the readouts. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD",
            meta = (ClampMin = "0.5"))
  float TextScale = 1.5f;

 private:
  void DrawReticle();
  void DrawTargetMarkers();
  void DrawReadouts();
};
