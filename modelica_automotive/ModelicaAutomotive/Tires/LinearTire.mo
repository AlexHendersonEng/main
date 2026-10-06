within ModelicaAutomotive.Tires;
block LinearTire "Linear slip stiffness with combined friction-circle saturation"
  extends ModelicaAutomotive.Interfaces.PartialTire;
  parameter ModelicaAutomotive.Types.LinearTireParameters parameters;
  ModelicaAutomotive.Tires.CombinedSlipLimiter limiter;
equation
  assert(parameters.longitudinalStiffness >= 0,
    "longitudinalStiffness must not be negative");
  assert(parameters.corneringStiffness >= 0,
    "corneringStiffness must not be negative");
  limiter.requestedLongitudinalForce =
    parameters.longitudinalStiffness * slipRatio;
  limiter.requestedLateralForce = parameters.corneringStiffness * slipAngle;
  limiter.normalLoad = normalLoad;
  limiter.frictionCoefficient = frictionCoefficient;
  longitudinalForce = limiter.longitudinalForce;
  lateralForce = limiter.lateralForce;
  utilization = limiter.utilization;
end LinearTire;
