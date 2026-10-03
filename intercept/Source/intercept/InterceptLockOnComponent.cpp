#include "InterceptLockOnComponent.h"

#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "InterceptTarget.h"

UInterceptLockOnComponent::UInterceptLockOnComponent() {
  PrimaryComponentTick.bCanEverTick = true;
}

void UInterceptLockOnComponent::GetAim(FVector& OutLocation,
                                       FVector& OutDirection) const {
  const AActor* Owner = GetOwner();
  FRotator Rotation;
  if (const APawn* Pawn = Cast<APawn>(Owner); Pawn && Pawn->GetController()) {
    // Controller view is the camera's aim; the pawn's own rotation ignores
    // pitch.
    Pawn->GetController()->GetPlayerViewPoint(OutLocation, Rotation);
  } else {
    Owner->GetActorEyesViewPoint(OutLocation, Rotation);
  }
  OutDirection = Rotation.Vector();
}

bool UInterceptLockOnComponent::IsTargetValid(const AInterceptTarget* Target,
                                              const FVector& Origin) const {
  if (!IsValid(Target)) {
    return false;
  }
  const FVector TargetLocation = Target->GetActorLocation();
  if (FVector::DistSquared(Origin, TargetLocation) >
      FMath::Square(MaxLockRange)) {
    return false;
  }
  if (bRequireLineOfSight) {
    FCollisionQueryParams Params(SCENE_QUERY_STAT(LockOnLOS), false,
                                 GetOwner());
    Params.AddIgnoredActor(Target);
    if (GetWorld()->LineTraceTestByChannel(Origin, TargetLocation,
                                           ECC_Visibility, Params)) {
      return false;
    }
  }
  return true;
}

AInterceptTarget* UInterceptLockOnComponent::FindBestTarget(
    const AInterceptTarget* Exclude) const {
  FVector Origin, Aim;
  GetAim(Origin, Aim);

  // Compare cosines instead of angles: larger cosine = smaller angle, and
  // avoids an acos per target.
  const float MinCos = FMath::Cos(FMath::DegreesToRadians(LockConeHalfAngle));
  float BestCos = MinCos;
  AInterceptTarget* Best = nullptr;

  for (TActorIterator<AInterceptTarget> It(GetWorld()); It; ++It) {
    AInterceptTarget* Candidate = *It;
    if (Candidate == Exclude) {
      continue;
    }
    const FVector ToTarget =
        (Candidate->GetActorLocation() - Origin).GetSafeNormal();
    const float CosAngle = FVector::DotProduct(Aim, ToTarget);
    if (CosAngle >= BestCos && IsTargetValid(Candidate, Origin)) {
      BestCos = CosAngle;
      Best = Candidate;
    }
  }
  return Best;
}

void UInterceptLockOnComponent::SetLock(AInterceptTarget* NewTarget) {
  if (LockedTarget != NewTarget) {
    LockedTarget = NewTarget;
    OnLockChanged.Broadcast(LockedTarget);
  }
}

void UInterceptLockOnComponent::ToggleLock() {
  if (LockedTarget) {
    ClearLock();
  } else {
    SetLock(FindBestTarget(nullptr));
  }
}

void UInterceptLockOnComponent::CycleTarget() {
  if (AInterceptTarget* Next = FindBestTarget(LockedTarget)) {
    SetLock(Next);
  }
}

void UInterceptLockOnComponent::ClearLock() { SetLock(nullptr); }

void UInterceptLockOnComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (!LockedTarget) {
    return;
  }

  // A destroyed target nulls LockedTarget via the UPROPERTY, but a pending-kill
  // one is caught by IsValid.
  FVector Origin, Aim;
  GetAim(Origin, Aim);
  if (!IsTargetValid(LockedTarget, Origin)) {
    ClearLock();
  }
}
