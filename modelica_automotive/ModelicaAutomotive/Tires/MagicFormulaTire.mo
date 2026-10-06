within ModelicaAutomotive.Tires;
block MagicFormulaTire
  "Compact parameterized pure-slip Magic Formula with combined-slip saturation"
  extends ModelicaAutomotive.Interfaces.PartialTire;
  parameter ModelicaAutomotive.Types.MagicFormulaTireParameters parameters;
  ModelicaAutomotive.Tires.CombinedSlipLimiter limiter;
protected
  Real peakForce(unit="N");
  Real longitudinalStiffnessFactor;
  Real lateralStiffnessFactor;
  Real longitudinalArgument;
  Real lateralArgument;
  Real pureLongitudinalForce(unit="N");
  Real pureLateralForce(unit="N");
equation
  assert(parameters.longitudinalStiffness > 0,
    "longitudinalStiffness must be positive");
  assert(parameters.lateralStiffness > 0,
    "lateralStiffness must be positive");
  assert(parameters.longitudinalShape > 0,
    "longitudinalShape must be positive");
  assert(parameters.lateralShape > 0, "lateralShape must be positive");
  assert(frictionCoefficient >= 0, "frictionCoefficient must not be negative");
  peakForce = frictionCoefficient * max(normalLoad, 0);
  longitudinalStiffnessFactor =
    if peakForce > ModelicaAutomotive.Constants.small then
      parameters.longitudinalStiffness
      / (parameters.longitudinalShape * peakForce) else 0;
  lateralStiffnessFactor =
    if peakForce > ModelicaAutomotive.Constants.small then
      parameters.lateralStiffness
      / (parameters.lateralShape * peakForce) else 0;
  longitudinalArgument = longitudinalStiffnessFactor * slipRatio;
  lateralArgument = lateralStiffnessFactor * tan(slipAngle);
  pureLongitudinalForce =
    if peakForce > ModelicaAutomotive.Constants.small then
      peakForce * sin(parameters.longitudinalShape * atan(
        longitudinalArgument
        - parameters.longitudinalCurvature
          * (longitudinalArgument - atan(longitudinalArgument)))) else 0;
  pureLateralForce =
    if peakForce > ModelicaAutomotive.Constants.small then
      peakForce * sin(parameters.lateralShape * atan(
        lateralArgument
        - parameters.lateralCurvature
          * (lateralArgument - atan(lateralArgument)))) else 0;
  limiter.requestedLongitudinalForce = pureLongitudinalForce;
  limiter.requestedLateralForce = pureLateralForce;
  limiter.normalLoad = normalLoad;
  limiter.frictionCoefficient = frictionCoefficient;
  longitudinalForce = limiter.longitudinalForce;
  lateralForce = limiter.lateralForce;
  utilization = limiter.utilization;
end MagicFormulaTire;
