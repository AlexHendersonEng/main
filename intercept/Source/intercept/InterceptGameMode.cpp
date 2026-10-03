#include "InterceptGameMode.h"

#include "InterceptPlayerPawn.h"

AInterceptGameMode::AInterceptGameMode() {
  // Native default; a Blueprint subclass of this game mode can override it with
  // a BP pawn.
  DefaultPawnClass = AInterceptPlayerPawn::StaticClass();
}
