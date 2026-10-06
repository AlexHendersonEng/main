within ModelicaAutomotive.Powertrain;
block MappedTorqueSource "MSL speed table with normalized drive command"
  parameter Real torqueMap[:, 2] = [
    0, 300;
    300, 300;
    600, 0]
    "Absolute angular speed and maximum drive torque";
  ModelicaAutomotive.Interfaces.RealInput command;
  ModelicaAutomotive.Interfaces.RealInput angularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput torque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput mechanicalPower(unit="W");
protected
  Modelica.Blocks.Tables.CombiTable1Ds table(
    table=torqueMap,
    columns={2},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=Modelica.Blocks.Types.Extrapolation.HoldLastPoint);
equation
  assert(size(torqueMap, 1) >= 2,
    "torqueMap must contain at least two rows");
  assert(torqueMap[1, 1] >= 0,
    "torqueMap speed values must not be negative");
  for row in 2:size(torqueMap, 1) loop
    assert(torqueMap[row, 1] > torqueMap[row - 1, 1],
      "torqueMap speed values must be strictly increasing");
  end for;
  table.u = abs(angularVelocity);
  torque = min(max(command, 0), 1) * table.y[1];
  mechanicalPower = torque * angularVelocity;
end MappedTorqueSource;
