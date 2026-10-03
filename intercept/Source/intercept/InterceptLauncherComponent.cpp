#include "InterceptLauncherComponent.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "InterceptInterceptor.h"
#include "InterceptLockOnComponent.h"
#include "InterceptTarget.h"

UInterceptLauncherComponent::UInterceptLauncherComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  InterceptorClass = AInterceptInterceptor::StaticClass();
}

void UInterceptLauncherComponent::BeginPlay() {
  Super::BeginPlay();
  LockOn = GetOwner()->FindComponentByClass<UInterceptLockOnComponent>();
  SetAmmo(MaxAmmo);
}

void UInterceptLauncherComponent::SetAmmo(int32 NewAmmo) {
  NewAmmo = FMath::Clamp(NewAmmo, 0, MaxAmmo);
  if (NewAmmo != Ammo) {
    Ammo = NewAmmo;
    OnAmmoChanged.Broadcast(Ammo, MaxAmmo);
  }
}

void UInterceptLauncherComponent::AddAmmo(int32 Amount) {
  SetAmmo(Ammo + Amount);
}

void UInterceptLauncherComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // Regenerate one round per ReloadTime while below max.
  if (ReloadTime <= 0.f || Ammo >= MaxAmmo) {
    ReloadProgress = 0.f;
    return;
  }

  ReloadProgress += DeltaTime;
  if (ReloadProgress >= ReloadTime) {
    ReloadProgress -= ReloadTime;
    SetAmmo(Ammo + 1);
  }
}

bool UInterceptLauncherComponent::TryLaunch() {
  UWorld* World = GetWorld();
  AInterceptTarget* Target = LockOn ? LockOn->GetLockedTarget() : nullptr;
  if (!World || !InterceptorClass || !Target || Ammo <= 0) {
    return false;
  }
  if (World->GetTimeSeconds() - LastLaunchTime < Cooldown) {
    return false;
  }

  // Launch from the camera view so the rocket leaves where the player is
  // looking, then PN steers it.
  FVector ViewLocation;
  FRotator ViewRotation;
  APawn* Pawn = Cast<APawn>(GetOwner());
  if (Pawn && Pawn->GetController()) {
    Pawn->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);
  } else {
    GetOwner()->GetActorEyesViewPoint(ViewLocation, ViewRotation);
  }

  const FVector SpawnLocation =
      ViewLocation + ViewRotation.RotateVector(MuzzleOffset);

  FActorSpawnParameters Params;
  Params.Owner = GetOwner();
  Params.Instigator = Pawn;
  Params.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  AInterceptInterceptor* Rocket = World->SpawnActor<AInterceptInterceptor>(
      InterceptorClass, SpawnLocation, ViewRotation, Params);
  if (!Rocket) {
    return false;
  }

  // Inherit the shooter's velocity so launching while moving doesn't skew the
  // initial heading.
  Rocket->Launch(
      Target, ViewRotation.Vector() * LaunchSpeed + GetOwner()->GetVelocity());

  LastLaunchTime = World->GetTimeSeconds();
  SetAmmo(Ammo - 1);
  OnLaunched.Broadcast();
  return true;
}
