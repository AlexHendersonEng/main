within ModelicaAutomotive.Tires;
block CombinedSlipLimiter "Scale two force components to a friction circle"
  ModelicaAutomotive.Interfaces.RealInput requestedLongitudinalForce(unit="N");
  ModelicaAutomotive.Interfaces.RealInput requestedLateralForce(unit="N");
  ModelicaAutomotive.Interfaces.RealInput normalLoad(unit="N");
  ModelicaAutomotive.Interfaces.RealInput frictionCoefficient;
  ModelicaAutomotive.Interfaces.RealOutput longitudinalForce(unit="N");
  ModelicaAutomotive.Interfaces.RealOutput lateralForce(unit="N");
  ModelicaAutomotive.Interfaces.RealOutput scale;
  ModelicaAutomotive.Interfaces.RealOutput utilization;
protected
  Real requestedMagnitude(unit="N");
  Real forceLimit(unit="N");
equation
  assert(frictionCoefficient >= 0, "frictionCoefficient must not be negative");
  requestedMagnitude = sqrt(
    requestedLongitudinalForce * requestedLongitudinalForce
    + requestedLateralForce * requestedLateralForce);
  forceLimit = frictionCoefficient * max(normalLoad, 0);
  scale = if requestedMagnitude > forceLimit
      and requestedMagnitude > ModelicaAutomotive.Constants.small then
    forceLimit / requestedMagnitude else 1;
  longitudinalForce = scale * requestedLongitudinalForce;
  lateralForce = scale * requestedLateralForce;
  utilization = if forceLimit > ModelicaAutomotive.Constants.small then
    sqrt(
      longitudinalForce * longitudinalForce
      + lateralForce * lateralForce) / forceLimit else 0;
end CombinedSlipLimiter;
