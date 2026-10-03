#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "InterceptLockOnComponent.generated.h"

class AInterceptTarget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInterceptLockChanged,
                                            AInterceptTarget*, NewTarget);

/**
 * Lets the owning pawn lock onto an incoming target by looking at it.
 * Candidates are targets inside a view cone and range with a clear line of
 * sight; the one closest to the aim direction is chosen. The lock persists
 * while the target stays in range and visible, and is dropped if the target is
 * destroyed or leaves range. The launcher (Phase 5) reads GetLockedTarget().
 */
UCLASS(ClassGroup = (Intercept), meta = (BlueprintSpawnableComponent))
class INTERCEPT_API UInterceptLockOnComponent : public UActorComponent {
  GENERATED_BODY()

 public:
  UInterceptLockOnComponent();

  virtual void TickComponent(
      float DeltaTime, ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  /** Locks the best target in the view cone, or releases the current lock if
   * one exists. */
  UFUNCTION(BlueprintCallable, Category = "LockOn")
  void ToggleLock();

  /** Moves the lock to the next-best candidate (by angle from aim, excluding
   * the current one). */
  UFUNCTION(BlueprintCallable, Category = "LockOn")
  void CycleTarget();

  /** Drops the current lock, if any. */
  UFUNCTION(BlueprintCallable, Category = "LockOn")
  void ClearLock();

  /** The locked target, or null. */
  UFUNCTION(BlueprintPure, Category = "LockOn")
  AInterceptTarget* GetLockedTarget() const { return LockedTarget; }

  /** Broadcast whenever the lock changes; NewTarget is null when the lock is
   * released. */
  UPROPERTY(BlueprintAssignable, Category = "LockOn")
  FInterceptLockChanged OnLockChanged;

 protected:
  /** Half-angle of the acquisition cone around the aim direction (degrees). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn",
            meta = (ClampMin = "1", ClampMax = "90", Units = "deg"))
  float LockConeHalfAngle = 15.f;

  /** Maximum lock range (cm). Beyond this the lock is lost. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn",
            meta = (ClampMin = "0", Units = "cm"))
  float MaxLockRange = 30000.f;

  /** Require an unobstructed line of sight to acquire and keep a lock. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
  bool bRequireLineOfSight = true;

 private:
  /** Eye location and aim direction of the owner (controller view if
   * available). */
  void GetAim(FVector& OutLocation, FVector& OutDirection) const;

  /** True if Target is within range and (optionally) visible from Origin. */
  bool IsTargetValid(const AInterceptTarget* Target,
                     const FVector& Origin) const;

  /** Best candidate inside the cone by smallest angle to Aim, skipping Exclude.
   */
  AInterceptTarget* FindBestTarget(const AInterceptTarget* Exclude) const;

  void SetLock(AInterceptTarget* NewTarget);

  UPROPERTY()
  TObjectPtr<AInterceptTarget> LockedTarget;
};
