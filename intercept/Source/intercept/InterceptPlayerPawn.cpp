#include "InterceptPlayerPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InterceptLockOnComponent.h"

AInterceptPlayerPawn::AInterceptPlayerPawn() {
  GetCapsuleComponent()->InitCapsuleSize(40.f, 90.f);

  // Body yaw follows the camera so the strafe/forward axes in Move() match
  // where the player looks.
  bUseControllerRotationYaw = true;

  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  Camera->SetupAttachment(GetCapsuleComponent());
  // Eye height: 70cm above capsule centre (capsule half-height is 90cm).
  Camera->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
  Camera->bUsePawnControlRotation = true;

  GetCharacterMovement()->MaxWalkSpeed = 600.f;

  LockOn = CreateDefaultSubobject<UInterceptLockOnComponent>(TEXT("LockOn"));
}

void AInterceptPlayerPawn::PostInitializeComponents() {
  Super::PostInitializeComponents();
  BuildDefaultInput();
}

// Builds input assets in code so Phase 1 needs no content; can be replaced by
// data assets later. Called from PostInitializeComponents so the objects exist
// before input is bound at possession.
void AInterceptPlayerPawn::BuildDefaultInput() {
  auto MakeAction = [this](const TCHAR* Name, EInputActionValueType Type) {
    UInputAction* Action = NewObject<UInputAction>(this, Name);
    Action->ValueType = Type;
    return Action;
  };

  MoveAction = MakeAction(TEXT("IA_Move"), EInputActionValueType::Axis2D);
  LookAction = MakeAction(TEXT("IA_Look"), EInputActionValueType::Axis2D);
  JumpAction = MakeAction(TEXT("IA_Jump"), EInputActionValueType::Boolean);
  LockAction = MakeAction(TEXT("IA_Lock"), EInputActionValueType::Boolean);
  FireAction = MakeAction(TEXT("IA_Fire"), EInputActionValueType::Boolean);

  MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Default"));

  auto AddKey = [this](UInputAction* Action, const FKey& Key,
                       bool bNegate = false, bool bSwizzle = false) {
    FEnhancedActionKeyMapping& Mapping = MappingContext->MapKey(Action, Key);
    if (bSwizzle) {
      Mapping.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(this));
    }
    if (bNegate) {
      Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(this));
    }
  };

  // Axis2D mapping: a key only produces a 1D value on X. Forward/back must land
  // on Y, so W/S are swizzled (YXZ) to move the value from X to Y; negate gives
  // the opposite direction (S, A, mouse Y).
  AddKey(MoveAction, EKeys::W, false, true);
  AddKey(MoveAction, EKeys::S, true, true);
  AddKey(MoveAction, EKeys::D);
  AddKey(MoveAction, EKeys::A, true);

  AddKey(LookAction, EKeys::MouseX);
  // Mouse Y is negated so moving the mouse up looks up (UE pitch input is
  // inverted by default).
  AddKey(LookAction, EKeys::MouseY, true, true);

  AddKey(JumpAction, EKeys::SpaceBar);
  AddKey(LockAction, EKeys::RightMouseButton);
  AddKey(FireAction, EKeys::LeftMouseButton);
}

void AInterceptPlayerPawn::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  // The mapping context must be registered with the local player's subsystem to
  // take effect.
  if (APlayerController* PC = Cast<APlayerController>(GetController())) {
    if (auto* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PC->GetLocalPlayer())) {
      Subsystem->AddMappingContext(MappingContext, 0);
    }
  }

  if (auto* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
    EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this,
                    &AInterceptPlayerPawn::Move);
    EIC->BindAction(LookAction, ETriggerEvent::Triggered, this,
                    &AInterceptPlayerPawn::Look);
    EIC->BindAction(JumpAction, ETriggerEvent::Started, this,
                    &ACharacter::Jump);
    EIC->BindAction(JumpAction, ETriggerEvent::Completed, this,
                    &ACharacter::StopJumping);
    // Lock/fire are forwarded as delegates so gameplay components don't need to
    // know about input.
    EIC->BindAction(LockAction, ETriggerEvent::Started, this, [this]() {
      LockOn->ToggleLock();
      OnLockPressed.Broadcast();
    });
    EIC->BindAction(FireAction, ETriggerEvent::Started, this,
                    [this]() { OnFirePressed.Broadcast(); });
  }
}

void AInterceptPlayerPawn::Move(const FInputActionValue& Value) {
  const FVector2D Axis = Value.Get<FVector2D>();
  AddMovementInput(GetActorForwardVector(), Axis.Y);
  AddMovementInput(GetActorRightVector(), Axis.X);
}

void AInterceptPlayerPawn::Look(const FInputActionValue& Value) {
  const FVector2D Axis = Value.Get<FVector2D>();
  AddControllerYawInput(Axis.X);
  AddControllerPitchInput(Axis.Y);
}
