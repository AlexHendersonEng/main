#pragma once

#include "CoreMinimal.h"

/**
 * Proportional navigation (PN) guidance, kept free of UObject dependencies so
 * it can be unit tested. All vectors are world space; units are cm, cm/s and
 * cm/s^2.
 *
 * Definitions (m = missile, t = target):
 *   R  = Rt - Rm            relative position (line of sight, LOS)
 *   V  = Vt - Vm            relative velocity
 *   Omega = (R x V) / (R.R) LOS angular rate vector (rad/s)
 *   Vc = -(R.V) / |R|       closing speed (positive when range is decreasing)
 */
struct FInterceptGuidance {
  /** Inputs describing the engagement geometry. */
  struct FState {
    FVector MissilePosition = FVector::ZeroVector;
    FVector MissileVelocity = FVector::ZeroVector;
    FVector TargetPosition = FVector::ZeroVector;
    FVector TargetVelocity = FVector::ZeroVector;
    /** Target acceleration; only used when bAugmented is set. */
    FVector TargetAcceleration = FVector::ZeroVector;
  };

  /** Navigation constant N, typically 3-5. Higher reacts harder to LOS
   * rotation. */
  float NavigationConstant = 4.f;

  /** Limit on commanded lateral acceleration (cm/s^2), modelling the airframe's
   * turn capability. */
  float MaxLateralAcceleration = 50000.f;

  /** Add the augmented PN term (N/2 * target acceleration normal to LOS). */
  bool bAugmented = false;

  /**
   * True PN command: a = N * Vc * (Omega x Vm_hat).
   * Perpendicular to the missile velocity, so it only turns the missile and
   * never changes its speed. Returns zero when the range is degenerate or the
   * target is opening (Vc <= 0), since PN assumes the missile is closing; a
   * pure pursuit fallback is the caller's concern.
   */
  FVector ComputeAcceleration(const FState& State) const;
};
