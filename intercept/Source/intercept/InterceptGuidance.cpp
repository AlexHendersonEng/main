#include "InterceptGuidance.h"

FVector FInterceptGuidance::ComputeAcceleration(const FState& State) const {
  const FVector R = State.TargetPosition - State.MissilePosition;
  const FVector V = State.TargetVelocity - State.MissileVelocity;
  const float RangeSq = R.SizeSquared();
  const FVector MissileDir = State.MissileVelocity.GetSafeNormal();

  if (RangeSq < UE_SMALL_NUMBER || MissileDir.IsZero()) {
    return FVector::ZeroVector;
  }

  const float Range = FMath::Sqrt(RangeSq);
  const float ClosingSpeed = -FVector::DotProduct(R, V) / Range;
  if (ClosingSpeed <= 0.f) {
    return FVector::ZeroVector;
  }

  const FVector Omega = FVector::CrossProduct(R, V) / RangeSq;
  FVector Command = NavigationConstant * ClosingSpeed *
                    FVector::CrossProduct(Omega, MissileDir);

  if (bAugmented) {
    // Only the target acceleration component normal to the LOS affects the miss
    // distance.
    const FVector LosDir = R / Range;
    const FVector NormalAccel =
        State.TargetAcceleration -
        FVector::DotProduct(State.TargetAcceleration, LosDir) * LosDir;
    Command += 0.5f * NavigationConstant * NormalAccel;
  }

  return Command.GetClampedToMaxSize(MaxLateralAcceleration);
}
