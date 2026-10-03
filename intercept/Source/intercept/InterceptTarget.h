#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InterceptTarget.generated.h"

class UStaticMeshComponent;
class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInterceptTargetDestroyed,
                                             AInterceptTarget*, Target, AActor*,
                                             Killer);

/**
 * An incoming hostile object the player must intercept.
 * Flies on a ballistic path (constant velocity plus optional gravity) and has
 * health. Interceptor rockets (Phase 4) damage it via the standard UE damage
 * system (TakeDamage). Exposes velocity and acceleration so guidance laws can
 * read the target state.
 */
UCLASS()
class INTERCEPT_API AInterceptTarget : public AActor {
  GENERATED_BODY()

 public:
  AInterceptTarget();

  virtual void Tick(float DeltaSeconds) override;
  virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                           AController* EventInstigator,
                           AActor* DamageCauser) override;

  /** Current velocity in cm/s (world space). */
  UFUNCTION(BlueprintPure, Category = "Target")
  FVector GetTargetVelocity() const { return Velocity; }

  /** Current acceleration in cm/s^2 (gravity only); used by augmented
   * proportional navigation. */
  UFUNCTION(BlueprintPure, Category = "Target")
  FVector GetTargetAcceleration() const;

  /** Sets the launch velocity in cm/s (world space). */
  UFUNCTION(BlueprintCallable, Category = "Target")
  void SetTargetVelocity(const FVector& NewVelocity) { Velocity = NewVelocity; }

  /** Fired once when health reaches zero, before the actor is destroyed. Killer
   * may be null. */
  UPROPERTY(BlueprintAssignable, Category = "Target")
  FInterceptTargetDestroyed OnTargetDestroyed;

  /** Fired when the target reaches the ground or the defended point without
   * being destroyed. */
  UPROPERTY(BlueprintAssignable, Category = "Target")
  FInterceptTargetDestroyed OnTargetImpact;

 protected:
  virtual void BeginPlay() override;

  /** Collision sphere, root of the actor. Blocks nothing; overlap is used for
   * impact. */
  UPROPERTY(VisibleAnywhere, Category = "Target")
  TObjectPtr<USphereComponent> Collision;

  /** Visual mesh; assign a different mesh in a Blueprint subclass. */
  UPROPERTY(VisibleAnywhere, Category = "Target")
  TObjectPtr<UStaticMeshComponent> Mesh;

  /** Hit points; reaches zero => destroyed. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target",
            meta = (ClampMin = "1"))
  float MaxHealth = 100.f;

  /** Multiplier on world gravity. 0 = straight line flight, 1 = full ballistic
   * arc. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
  float GravityScale = 0.f;

  /** Score awarded for destroying this target (used by Phase 6). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
  int32 ScoreValue = 100;

 private:
  UFUNCTION()
  void HandleOverlap(UPrimitiveComponent* OverlappedComponent,
                     AActor* OtherActor, UPrimitiveComponent* OtherComp,
                     int32 OtherBodyIndex, bool bFromSweep,
                     const FHitResult& SweepResult);

  /** Ends the flight as an impact (not a kill). */
  void Impact(AActor* HitActor);

  FVector Velocity = FVector::ZeroVector;
  float Health = 0.f;
  bool bFinished = false;

 public:
  int32 GetScoreValue() const { return ScoreValue; }
};
