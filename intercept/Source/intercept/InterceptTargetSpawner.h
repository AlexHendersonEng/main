#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InterceptTargetSpawner.generated.h"

class AInterceptTarget;

/**
 * Spawns incoming targets at random points on a ring around the defended point
 * and launches them toward it. Placed in the level; Phase 6 replaces the fixed
 * timer with a wave manager.
 */
UCLASS()
class INTERCEPT_API AInterceptTargetSpawner : public AActor {
  GENERATED_BODY()

 public:
  AInterceptTargetSpawner();

  /** Spawns one target now. Returns it, or null if the class is unset or
   * spawning failed. */
  UFUNCTION(BlueprintCallable, Category = "Spawner")
  AInterceptTarget* SpawnTarget();

  /** Starts/stops periodic spawning. */
  UFUNCTION(BlueprintCallable, Category = "Spawner")
  void SetSpawningEnabled(bool bEnabled);

 protected:
  virtual void BeginPlay() override;

  /** Class to spawn; set a Blueprint subclass to change mesh/health. Defaults
   * to AInterceptTarget. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
  TSubclassOf<AInterceptTarget> TargetClass;

  /** Defended point, relative to this actor's location (cm). Targets are aimed
   * here. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
  FVector DefendedOffset = FVector::ZeroVector;

  /** Horizontal distance of the spawn ring from the defended point (cm). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner",
            meta = (ClampMin = "0", Units = "cm"))
  float SpawnRadius = 15000.f;

  /** Spawn height above the defended point (cm). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner",
            meta = (Units = "cm"))
  float SpawnHeight = 8000.f;

  /** Target speed (cm/s). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner",
            meta = (ClampMin = "0", Units = "cm/s"))
  float TargetSpeed = 1500.f;

  /** Seconds between spawns. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner",
            meta = (ClampMin = "0.1", Units = "s"))
  float SpawnInterval = 3.f;

  /** Begin spawning automatically on BeginPlay. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
  bool bAutoStart = true;

 private:
  FTimerHandle SpawnTimer;
};
