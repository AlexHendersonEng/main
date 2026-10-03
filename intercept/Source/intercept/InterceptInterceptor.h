#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InterceptGuidance.h"
#include "InterceptInterceptor.generated.h"

class AInterceptTarget;
class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Player-launched interceptor rocket guided by proportional navigation.
 * Each tick it asks FInterceptGuidance for a lateral acceleration, turns its
 * velocity, and accelerates along its heading until it reaches CruiseSpeed. It
 * explodes (radial damage) on impact, when inside the proximity fuse radius, or
 * when its lifetime or fuel runs out.
 */
UCLASS()
class INTERCEPT_API AInterceptInterceptor : public AActor {
  GENERATED_BODY()

 public:
  AInterceptInterceptor();

  virtual void Tick(float DeltaSeconds) override;

  /** Sets the target to chase and the initial velocity (cm/s). Call right after
   * spawning. */
  UFUNCTION(BlueprintCallable, Category = "Interceptor")
  void Launch(AInterceptTarget* InTarget, const FVector& InitialVelocity);

 protected:
  virtual void BeginPlay() override;

  UPROPERTY(VisibleAnywhere, Category = "Interceptor")
  TObjectPtr<USphereComponent> Collision;

  UPROPERTY(VisibleAnywhere, Category = "Interceptor")
  TObjectPtr<UStaticMeshComponent> Mesh;

  /** Point light that makes the rocket glow. */
  UPROPERTY(VisibleAnywhere, Category = "Interceptor")
  TObjectPtr<UPointLightComponent> GlowLight;

  /** Colour of the rocket body and its glow. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
  FLinearColor GlowColor = FLinearColor(0.1f, 0.8f, 1.f);

  /** Brightness of the glow light (0 turns it off). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals",
            meta = (ClampMin = "0"))
  float GlowIntensity = 15000.f;

  /** Speed the rocket accelerates to along its heading (cm/s). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interceptor",
            meta = (ClampMin = "0", Units = "cm/s"))
  float CruiseSpeed = 6000.f;

  /** Forward acceleration while below CruiseSpeed (cm/s^2). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interceptor",
            meta = (ClampMin = "0", Units = "cm/s^2"))
  float ThrustAcceleration = 8000.f;

  /** Navigation constant N (typically 3-5). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance",
            meta = (ClampMin = "1", ClampMax = "10"))
  float NavigationConstant = 4.f;

  /** Maximum lateral acceleration the airframe can sustain (cm/s^2). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance",
            meta = (ClampMin = "0", Units = "cm/s^2"))
  float MaxLateralAcceleration = 50000.f;

  /** Use augmented PN, which also reacts to target acceleration. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance")
  bool bAugmentedGuidance = true;

  /** Detonates when closer than this to the target (cm). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warhead",
            meta = (ClampMin = "0", Units = "cm"))
  float ProximityFuseRadius = 150.f;

  /** Radius of the explosion's damage (cm). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warhead",
            meta = (ClampMin = "0", Units = "cm"))
  float BlastRadius = 400.f;

  /** Damage at the explosion centre. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warhead",
            meta = (ClampMin = "0"))
  float BlastDamage = 100.f;

  /** Self-destructs after this many seconds. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interceptor",
            meta = (ClampMin = "0.1", Units = "s"))
  float MaxLifetime = 12.f;

  /** Draws the line of sight and commanded acceleration for this rocket. The
   * console variable intercept.DebugGuidance enables it for all rockets. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
  bool bDebugDraw = false;

 private:
  UFUNCTION()
  void HandleOverlap(UPrimitiveComponent* OverlappedComponent,
                     AActor* OtherActor, UPrimitiveComponent* OtherComp,
                     int32 OtherBodyIndex, bool bFromSweep,
                     const FHitResult& SweepResult);

  /** Applies radial damage and destroys the rocket. */
  void Detonate();

  UPROPERTY()
  TObjectPtr<AInterceptTarget> Target;

  FVector Velocity = FVector::ZeroVector;
  float Age = 0.f;
  bool bDetonated = false;
};
