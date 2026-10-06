within ModelicaAutomotive.Brakes;
block FrictionLimitedBrake "Brake torque limited by lining friction and clamp load"
  parameter ModelicaAutomotive.Types.Torque maximumCommandTorque = 5000;
  parameter Real liningFrictionCoefficient(min=0) = 0.4;
  parameter ModelicaAutomotive.Types.Length effectiveRadius = 0.15;
  parameter ModelicaAutomotive.Types.AngularVelocity speedRegularization = 0.1;
  ModelicaAutomotive.Interfaces.RealInput command;
  ModelicaAutomotive.Interfaces.RealInput clampForce(unit="N");
  ModelicaAutomotive.Interfaces.RealInput angularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput brakeTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput appliedMagnitude(unit="N.m");
equation
  assert(maximumCommandTorque >= 0,
    "maximumCommandTorque must not be negative");
  assert(liningFrictionCoefficient >= 0,
    "liningFrictionCoefficient must not be negative");
  assert(effectiveRadius > 0, "effectiveRadius must be positive");
  assert(speedRegularization > 0, "speedRegularization must be positive");
  appliedMagnitude = min(
    maximumCommandTorque * min(max(command, 0), 1),
    liningFrictionCoefficient * max(clampForce, 0) * effectiveRadius);
  brakeTorque = -appliedMagnitude * angularVelocity / sqrt(
    angularVelocity * angularVelocity
    + speedRegularization * speedRegularization);
end FrictionLimitedBrake;
