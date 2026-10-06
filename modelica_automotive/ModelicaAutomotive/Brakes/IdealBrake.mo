within ModelicaAutomotive.Brakes;
block IdealBrake "Commanded brake torque opposing wheel rotation"
  parameter ModelicaAutomotive.Types.Torque maximumTorque = 3000;
  parameter ModelicaAutomotive.Types.AngularVelocity speedRegularization = 0.1;
  ModelicaAutomotive.Interfaces.RealInput command
    "Normalized brake command";
  ModelicaAutomotive.Interfaces.RealInput angularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput brakeTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput appliedMagnitude(unit="N.m");
equation
  assert(maximumTorque >= 0, "maximumTorque must not be negative");
  assert(speedRegularization > 0, "speedRegularization must be positive");
  appliedMagnitude = maximumTorque * min(max(command, 0), 1);
  brakeTorque = -appliedMagnitude * angularVelocity / sqrt(
    angularVelocity * angularVelocity
    + speedRegularization * speedRegularization);
end IdealBrake;
