#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "InterceptLauncherComponent.generated.h"

class AInterceptInterceptor;
class UInterceptLockOnComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInterceptAmmoChanged, int32, Ammo,
                                             int32, MaxAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInterceptLaunched);

/**
 * Fires guided interceptors on behalf of the owning pawn.
 * A launch requires a locked target (from UInterceptLockOnComponent on the same
 * actor), ammo, and the cooldown to have elapsed. Ammo regenerates one round at
 * a time after a delay.
 */
UCLASS(ClassGroup = (Intercept), meta = (BlueprintSpawnableComponent))
class INTERCEPT_API UInterceptLauncherComponent : public UActorComponent {
  GENERATED_BODY()

 public:
  UInterceptLauncherComponent();

  /** Launches an interceptor at the locked target. Returns false if there is no
   * lock, no ammo, or the launcher is cooling down. */
  UFUNCTION(BlueprintCallable, Category = "Launcher")
  bool TryLaunch();

  /** Adds rounds, up to MaxAmmo (e.g. a pickup or between waves). */
  UFUNCTION(BlueprintCallable, Category = "Launcher")
  void AddAmmo(int32 Amount);

  UFUNCTION(BlueprintPure, Category = "Launcher")
  int32 GetAmmo() const { return Ammo; }

  UFUNCTION(BlueprintPure, Category = "Launcher")
  int32 GetMaxAmmo() const { return MaxAmmo; }

  /** Broadcast whenever the ammo count changes. */
  UPROPERTY(BlueprintAssignable, Category = "Launcher")
  FInterceptAmmoChanged OnAmmoChanged;

  /** Broadcast after each successful launch (hook for sound/VFX). */
  UPROPERTY(BlueprintAssignable, Category = "Launcher")
  FInterceptLaunched OnLaunched;

 protected:
  virtual void BeginPlay() override;
  virtual void TickComponent(
      float DeltaTime, ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  /** Interceptor class to spawn; set a Blueprint subclass for custom
   * visuals/tuning. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher")
  TSubclassOf<AInterceptInterceptor> InterceptorClass;

  /** Maximum rounds carried. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher",
            meta = (ClampMin = "0"))
  int32 MaxAmmo = 6;

  /** Minimum time between launches (s). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher",
            meta = (ClampMin = "0", Units = "s"))
  float Cooldown = 0.75f;

  /** Seconds to regenerate one round; 0 disables regeneration. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher",
            meta = (ClampMin = "0", Units = "s"))
  float ReloadTime = 4.f;

  /** Muzzle position relative to the owner's view point (cm), in view space: X
   * forward, Y right, Z up. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher")
  FVector MuzzleOffset = FVector(100.f, 30.f, -20.f);

  /** Initial rocket speed (cm/s); the rocket then thrusts to its cruise speed.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher",
            meta = (ClampMin = "0", Units = "cm/s"))
  float LaunchSpeed = 2000.f;

 private:
  void SetAmmo(int32 NewAmmo);

  UPROPERTY()
  TObjectPtr<UInterceptLockOnComponent> LockOn;

  int32 Ammo = 0;
  double LastLaunchTime = -1000.0;
  float ReloadProgress = 0.f;
};
