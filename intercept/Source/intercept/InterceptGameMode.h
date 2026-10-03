#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InterceptGameMode.generated.h"

/**
 * Game mode for the interceptor game.
 * Selects the player pawn; later phases add wave management, scoring and
 * game-over rules.
 */
UCLASS()
class INTERCEPT_API AInterceptGameMode : public AGameModeBase {
  GENERATED_BODY()

 public:
  AInterceptGameMode();
};
