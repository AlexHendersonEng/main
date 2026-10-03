#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InterceptGameMode.generated.h"

class AInterceptTarget;
class AInterceptTargetSpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInterceptIntChanged, int32,
                                            NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInterceptGameOver);

/**
 * Game mode for the interceptor game.
 * Selects the player pawn and runs the rules: targets arrive in waves from the
 * level's AInterceptTargetSpawner, destroying one scores points, and a target
 * that reaches the defended area costs a life. Each wave is bigger and faster
 * than the last. When lives run out the game ends and the level restarts after
 * a short delay. The HUD (Phase 7) binds to the delegates below.
 */
UCLASS()
class INTERCEPT_API AInterceptGameMode : public AGameModeBase {
  GENERATED_BODY()

 public:
  AInterceptGameMode();

  UFUNCTION(BlueprintPure, Category = "Rules")
  int32 GetScore() const { return Score; }

  UFUNCTION(BlueprintPure, Category = "Rules")
  int32 GetLives() const { return Lives; }

  UFUNCTION(BlueprintPure, Category = "Rules")
  int32 GetWave() const { return Wave; }

  UFUNCTION(BlueprintPure, Category = "Rules")
  bool IsGameOver() const { return bGameOver; }

  /** Broadcast with the new score whenever it changes. */
  UPROPERTY(BlueprintAssignable, Category = "Rules")
  FInterceptIntChanged OnScoreChanged;

  /** Broadcast with the remaining lives whenever they change. */
  UPROPERTY(BlueprintAssignable, Category = "Rules")
  FInterceptIntChanged OnLivesChanged;

  /** Broadcast with the wave number when a wave starts. */
  UPROPERTY(BlueprintAssignable, Category = "Rules")
  FInterceptIntChanged OnWaveStarted;

  /** Broadcast once when the last life is lost. */
  UPROPERTY(BlueprintAssignable, Category = "Rules")
  FInterceptGameOver OnGameOver;

 protected:
  virtual void BeginPlay() override;

  /** Lives at the start; one is lost per target that gets through. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules",
            meta = (ClampMin = "1"))
  int32 StartingLives = 5;

  /** Number of targets in wave 1. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves",
            meta = (ClampMin = "1"))
  int32 FirstWaveTargets = 3;

  /** Extra targets added to each following wave. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves",
            meta = (ClampMin = "0"))
  int32 TargetsPerWaveIncrease = 2;

  /** Fractional target speed increase per wave (0.15 = +15% each wave). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves",
            meta = (ClampMin = "0"))
  float SpeedRampPerWave = 0.15f;

  /** Seconds between targets within a wave. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves",
            meta = (ClampMin = "0.1", Units = "s"))
  float SpawnInterval = 2.5f;

  /** Seconds of calm after a wave is cleared before the next starts. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves",
            meta = (ClampMin = "0", Units = "s"))
  float TimeBetweenWaves = 5.f;

  /** Interceptors given to the player when a wave is cleared. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves",
            meta = (ClampMin = "0"))
  int32 AmmoRewardPerWave = 3;

  /** Seconds after game over before the level restarts. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules",
            meta = (ClampMin = "0", Units = "s"))
  float RestartDelay = 5.f;

 private:
  void StartNextWave();
  void SpawnWaveTarget();

  /** Called when a target is destroyed by the player. */
  UFUNCTION()
  void HandleTargetDestroyed(AInterceptTarget* Target, AActor* Killer);

  /** Called when a target reaches the defended area. */
  UFUNCTION()
  void HandleTargetImpact(AInterceptTarget* Target, AActor* HitActor);

  /** Common bookkeeping for a target that is no longer in play. */
  void OnTargetResolved();

  void EndGame();
  void RestartLevel();

  UPROPERTY()
  TObjectPtr<AInterceptTargetSpawner> Spawner;

  FTimerHandle SpawnTimer;
  FTimerHandle NextWaveTimer;
  FTimerHandle RestartTimer;

  int32 Score = 0;
  int32 Lives = 0;
  int32 Wave = 0;
  /** Targets in the current wave still to be spawned. */
  int32 PendingSpawns = 0;
  /** Targets in the current wave that are alive or still to be spawned. */
  int32 TargetsRemaining = 0;
  bool bGameOver = false;
};
