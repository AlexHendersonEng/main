within ModelicaAutomotive.Brakes;
block MappedBrake "MSL table-mapped brake command with signed output torque"
  parameter Real torqueMap[:, 2] = [0, 0; 0.5, 1200; 1, 3000]
    "Normalized command and brake torque magnitude";
  parameter ModelicaAutomotive.Types.AngularVelocity speedRegularization = 0.1;
  ModelicaAutomotive.Interfaces.RealInput command;
  ModelicaAutomotive.Interfaces.RealInput angularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput brakeTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput appliedMagnitude(unit="N.m");
protected
  Modelica.Blocks.Tables.CombiTable1Ds table(
    table=torqueMap,
    columns={2},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=Modelica.Blocks.Types.Extrapolation.HoldLastPoint);
equation
  assert(speedRegularization > 0, "speedRegularization must be positive");
  table.u = min(max(command, torqueMap[1, 1]), torqueMap[size(torqueMap, 1), 1]);
  appliedMagnitude = table.y[1];
  brakeTorque = -appliedMagnitude * angularVelocity / sqrt(
    angularVelocity * angularVelocity
    + speedRegularization * speedRegularization);
end MappedBrake;
