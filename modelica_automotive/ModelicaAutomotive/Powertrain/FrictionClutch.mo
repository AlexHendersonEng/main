within ModelicaAutomotive.Powertrain;
block FrictionClutch "Regularized friction clutch torque and dissipation"
  parameter ModelicaAutomotive.Types.Torque maximumTorque = 500;
  parameter ModelicaAutomotive.Types.AngularVelocity slipRegularization = 0.5;
  ModelicaAutomotive.Interfaces.RealInput command;
  ModelicaAutomotive.Interfaces.RealInput inputAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealInput outputAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput inputTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput outputTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput dissipatedPower(unit="W");
protected
  Real slipSpeed(unit="rad/s");
  Real torqueMagnitude(unit="N.m");
equation
  assert(maximumTorque >= 0, "maximumTorque must not be negative");
  assert(slipRegularization > 0, "slipRegularization must be positive");
  slipSpeed = inputAngularVelocity - outputAngularVelocity;
  torqueMagnitude = maximumTorque * min(max(command, 0), 1);
  inputTorque = -torqueMagnitude * slipSpeed / sqrt(
    slipSpeed * slipSpeed + slipRegularization * slipRegularization);
  outputTorque = -inputTorque;
  dissipatedPower = -inputTorque * slipSpeed;
end FrictionClutch;
