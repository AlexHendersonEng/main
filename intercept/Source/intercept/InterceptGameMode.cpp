#include "InterceptGameMode.h"

#include "EngineUtils.h"
#include "InterceptHUD.h"
#include "InterceptLauncherComponent.h"
#include "InterceptPlayerPawn.h"
#include "InterceptTarget.h"
#include "InterceptTargetSpawner.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AInterceptGameMode::AInterceptGameMode() {
  // Native default; a Blueprint subclass of this game mode can override it with
  // a BP pawn.
  DefaultPawnClass = AInterceptPlayerPawn::StaticClass();
  HUDClass = AInterceptHUD::StaticClass();
}

void AInterceptGameMode::BeginPlay() {
  Super::BeginPlay();

  Lives = StartingLives;
  OnLivesChanged.Broadcast(Lives);
  OnScoreChanged.Broadcast(Score);

  for (TActorIterator<AInterceptTargetSpawner> It(GetWorld()); It; ++It) {
    Spawner = *It;
    break;
  }
  if (!Spawner) {
    UE_LOG(LogTemp, Warning,
           TEXT("AInterceptGameMode: no AInterceptTargetSpawner in the level; "
                "no waves will run."));
    return;
  }

  StartNextWave();
}

void AInterceptGameMode::StartNextWave() {
  ++Wave;
  PendingSpawns = FirstWaveTargets + (Wave - 1) * TargetsPerWaveIncrease;
  TargetsRemaining = PendingSpawns;
  OnWaveStarted.Broadcast(Wave);

  GetWorldTimerManager().SetTimer(
      SpawnTimer,
      FTimerDelegate::CreateUObject(this, &AInterceptGameMode::SpawnWaveTarget),
      SpawnInterval, true, 0.f);
}

void AInterceptGameMode::SpawnWaveTarget() {
  if (PendingSpawns <= 0 || bGameOver) {
    GetWorldTimerManager().ClearTimer(SpawnTimer);
    return;
  }

  const float SpeedMultiplier = 1.f + SpeedRampPerWave * (Wave - 1);
  AInterceptTarget* Target = Spawner->SpawnTarget(SpeedMultiplier);
  --PendingSpawns;

  if (Target) {
    Target->OnTargetDestroyed.AddDynamic(
        this, &AInterceptGameMode::HandleTargetDestroyed);
    Target->OnTargetImpact.AddDynamic(this,
                                      &AInterceptGameMode::HandleTargetImpact);
  } else {
    // A failed spawn must not leave the wave waiting for a target that never
    // existed.
    OnTargetResolved();
  }

  if (PendingSpawns <= 0) {
    GetWorldTimerManager().ClearTimer(SpawnTimer);
  }
}

void AInterceptGameMode::HandleTargetDestroyed(AInterceptTarget* Target,
                                               AActor* Killer) {
  if (bGameOver) {
    return;
  }
  Score += Target->GetScoreValue();
  OnScoreChanged.Broadcast(Score);
  OnTargetResolved();
}

void AInterceptGameMode::HandleTargetImpact(AInterceptTarget* Target,
                                            AActor* HitActor) {
  if (bGameOver) {
    return;
  }
  Lives = FMath::Max(Lives - 1, 0);
  OnLivesChanged.Broadcast(Lives);

  if (Lives == 0) {
    EndGame();
    return;
  }
  OnTargetResolved();
}

void AInterceptGameMode::OnTargetResolved() {
  --TargetsRemaining;
  if (TargetsRemaining > 0 || PendingSpawns > 0 || bGameOver) {
    return;
  }

  // Wave cleared: resupply the player and queue the next wave.
  if (auto* Pawn = Cast<AInterceptPlayerPawn>(
          UGameplayStatics::GetPlayerPawn(this, 0))) {
    Pawn->GetLauncher()->AddAmmo(AmmoRewardPerWave);
  }
  GetWorldTimerManager().SetTimer(
      NextWaveTimer,
      FTimerDelegate::CreateUObject(this, &AInterceptGameMode::StartNextWave),
      FMath::Max(TimeBetweenWaves, 0.01f), false);
}

void AInterceptGameMode::EndGame() {
  bGameOver = true;
  GetWorldTimerManager().ClearTimer(SpawnTimer);
  GetWorldTimerManager().ClearTimer(NextWaveTimer);
  OnGameOver.Broadcast();

  GetWorldTimerManager().SetTimer(
      RestartTimer,
      FTimerDelegate::CreateUObject(this, &AInterceptGameMode::RestartLevel),
      FMath::Max(RestartDelay, 0.01f), false);
}

void AInterceptGameMode::RestartLevel() {
  UGameplayStatics::OpenLevel(
      this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}
