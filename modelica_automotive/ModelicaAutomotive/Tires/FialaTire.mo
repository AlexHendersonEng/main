within ModelicaAutomotive.Tires;
block FialaTire "Fiala-style brush tire with combined-slip saturation"
  extends ModelicaAutomotive.Interfaces.PartialTire;
  parameter ModelicaAutomotive.Types.FialaTireParameters parameters;
  ModelicaAutomotive.Tires.CombinedSlipLimiter limiter;
protected
  Real forceLimit(unit="N");
  Real longitudinalCritical;
  Real lateralCritical;
  Real lateralSlip;
  Real pureLongitudinalForce(unit="N");
  Real pureLateralForce(unit="N");
equation
  assert(parameters.longitudinalStiffness > 0,
    "longitudinalStiffness must be positive");
  assert(parameters.corneringStiffness > 0,
    "corneringStiffness must be positive");
  assert(frictionCoefficient >= 0, "frictionCoefficient must not be negative");
  forceLimit = frictionCoefficient * max(normalLoad, 0);
  longitudinalCritical = 3 * forceLimit / parameters.longitudinalStiffness;
  lateralCritical = 3 * forceLimit / parameters.corneringStiffness;
  lateralSlip = tan(slipAngle);
  pureLongitudinalForce =
    if forceLimit <= ModelicaAutomotive.Constants.small then 0
    elseif abs(slipRatio) < longitudinalCritical then
      parameters.longitudinalStiffness * slipRatio
      - parameters.longitudinalStiffness ^ 2
        * abs(slipRatio) * slipRatio / (3 * forceLimit)
      + parameters.longitudinalStiffness ^ 3 * slipRatio ^ 3
        / (27 * forceLimit ^ 2)
    else forceLimit * ModelicaAutomotive.Mathematics.regularizedSign(
      slipRatio,
      ModelicaAutomotive.Constants.small);
  pureLateralForce =
    if forceLimit <= ModelicaAutomotive.Constants.small then 0
    elseif abs(lateralSlip) < lateralCritical then
      parameters.corneringStiffness * lateralSlip
      - parameters.corneringStiffness ^ 2
        * abs(lateralSlip) * lateralSlip / (3 * forceLimit)
      + parameters.corneringStiffness ^ 3 * lateralSlip ^ 3
        / (27 * forceLimit ^ 2)
    else forceLimit * ModelicaAutomotive.Mathematics.regularizedSign(
      lateralSlip,
      ModelicaAutomotive.Constants.small);
  limiter.requestedLongitudinalForce = pureLongitudinalForce;
  limiter.requestedLateralForce = pureLateralForce;
  limiter.normalLoad = normalLoad;
  limiter.frictionCoefficient = frictionCoefficient;
  longitudinalForce = limiter.longitudinalForce;
  lateralForce = limiter.lateralForce;
  utilization = limiter.utilization;
end FialaTire;
