within ModelicaAutomotive.Brakes;
block FirstOrderBrake "First-order brake actuation with signed output torque"
  parameter ModelicaAutomotive.Types.Torque maximumTorque = 3000;
  parameter Real timeConstant(unit="s") = 0.1;
  parameter Real initialApplication(min=0, max=1) = 0;
  parameter ModelicaAutomotive.Types.AngularVelocity speedRegularization = 0.1;
  ModelicaAutomotive.Interfaces.RealInput command;
  ModelicaAutomotive.Interfaces.RealInput angularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput brakeTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput application;
protected
  Real applicationState(start=initialApplication, fixed=true);
equation
  assert(maximumTorque >= 0, "maximumTorque must not be negative");
  assert(timeConstant > 0, "timeConstant must be positive");
  assert(speedRegularization > 0, "speedRegularization must be positive");
  der(applicationState) =
    (min(max(command, 0), 1) - applicationState) / timeConstant;
  application = applicationState;
  brakeTorque = -maximumTorque * applicationState * angularVelocity / sqrt(
    angularVelocity * angularVelocity
    + speedRegularization * speedRegularization);
end FirstOrderBrake;
