#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InterceptPlayerPawn.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UInterceptLockOnComponent;
class UInterceptLauncherComponent;
struct FInputActionValue;

/**
 * First-person player character.
 * Handles movement and look input and exposes lock-on / fire input as
 * delegates, so the lock-on component and launcher (later phases) can bind
 * without depending on input details.
 */
UCLASS()
class INTERCEPT_API AInterceptPlayerPawn : public ACharacter {
  GENERATED_BODY()

 public:
  AInterceptPlayerPawn();

  virtual void SetupPlayerInputComponent(
      UInputComponent* PlayerInputComponent) override;

  /** Default input mapping context created in code (see BuildDefaultInput). */
  UInputMappingContext* GetMappingContext() const { return MappingContext; }

  /** Lock-on component; its lock is toggled by the lock key. */
  UInterceptLockOnComponent* GetLockOn() const { return LockOn; }

  /** Launcher component; fires at the locked target when the fire key is
   * pressed. */
  UInterceptLauncherComponent* GetLauncher() const { return Launcher; }

  DECLARE_MULTICAST_DELEGATE(FInterceptInputEvent);

  /** Broadcast when the lock-on key is pressed (right mouse button by default).
   */
  FInterceptInputEvent OnLockPressed;

  /** Broadcast when the fire key is pressed (left mouse button by default). */
  FInterceptInputEvent OnFirePressed;

 protected:
  virtual void PostInitializeComponents() override;

  /** Applies a 2D move axis: X = strafe right, Y = forward, relative to the
   * actor's facing. */
  void Move(const FInputActionValue& Value);

  /** Applies a 2D look axis to the controller: X = yaw, Y = pitch. */
  void Look(const FInputActionValue& Value);

  /** First-person camera at eye height; follows the controller rotation. */
  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UCameraComponent> Camera;

  /** Acquires and tracks the locked target. */
  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UInterceptLockOnComponent> LockOn;

  /** Spawns guided interceptors and manages ammo. */
  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UInterceptLauncherComponent> Launcher;

  /** Maps keys to the actions below. Runtime-created, so UPROPERTY keeps it
   * from being garbage collected. */
  UPROPERTY()
  TObjectPtr<UInputMappingContext> MappingContext;

  UPROPERTY()
  TObjectPtr<UInputAction> MoveAction;
  UPROPERTY()
  TObjectPtr<UInputAction> LookAction;
  UPROPERTY()
  TObjectPtr<UInputAction> JumpAction;
  UPROPERTY()
  TObjectPtr<UInputAction> LockAction;
  UPROPERTY()
  TObjectPtr<UInputAction> FireAction;

 private:
  /** Creates the input actions and key mappings in code so no content assets
   * are required yet. */
  void BuildDefaultInput();
};
