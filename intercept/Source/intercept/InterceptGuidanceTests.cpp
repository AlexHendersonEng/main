#include "InterceptGuidance.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace {
/** Steps a missile (PN guided) against a target with constant velocity; returns
 * the minimum miss distance in cm. */
float SimulateEngagement(const FInterceptGuidance& Guidance, FVector TargetPos,
                         const FVector& TargetVel, float MissileSpeed,
                         FVector MissilePos, FVector MissileDir) {
  const float Dt = 1.f / 120.f;
  FVector MissileVel = MissileDir.GetSafeNormal() * MissileSpeed;
  float MinDist = FVector::Dist(TargetPos, MissilePos);

  for (float T = 0.f; T < 30.f; T += Dt) {
    FInterceptGuidance::FState State;
    State.MissilePosition = MissilePos;
    State.MissileVelocity = MissileVel;
    State.TargetPosition = TargetPos;
    State.TargetVelocity = TargetVel;

    // Rotate velocity by the lateral acceleration while holding speed constant.
    MissileVel = (MissileVel + Guidance.ComputeAcceleration(State) * Dt)
                     .GetSafeNormal() *
                 MissileSpeed;
    MissilePos += MissileVel * Dt;
    TargetPos += TargetVel * Dt;

    MinDist = FMath::Min(MinDist, FVector::Dist(TargetPos, MissilePos));
  }
  return MinDist;
}
}  // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInterceptGuidanceCrossingTest,
                                 "Intercept.Guidance.CrossingTarget",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::ProductFilter)

bool FInterceptGuidanceCrossingTest::RunTest(const FString& Parameters) {
  FInterceptGuidance Guidance;

  // Target crosses the missile's path at 1000 cm/s; missile (3000 cm/s) starts
  // pointed 40 degrees off.
  const float Miss = SimulateEngagement(
      Guidance, FVector(10000, 5000, 0), FVector(0, -1000, 0), 3000.f,
      FVector::ZeroVector, FVector(1, 0.8f, 0));
  TestTrue(FString::Printf(TEXT("Miss distance %.1f cm should be under 100 cm"),
                           Miss),
           Miss < 100.f);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInterceptGuidanceOpeningTest,
                                 "Intercept.Guidance.NoCommandWhenOpening",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::ProductFilter)

bool FInterceptGuidanceOpeningTest::RunTest(const FString& Parameters) {
  FInterceptGuidance Guidance;
  FInterceptGuidance::FState State;
  State.TargetPosition = FVector(1000, 0, 0);
  State.TargetVelocity = FVector(2000, 0, 0);
  State.MissileVelocity = FVector(1000, 0, 0);

  TestTrue(TEXT("Zero command while range is opening"),
           Guidance.ComputeAcceleration(State).IsZero());
  return true;
}

#endif
