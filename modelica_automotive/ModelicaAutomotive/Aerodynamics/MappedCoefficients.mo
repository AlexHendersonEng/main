within ModelicaAutomotive.Aerodynamics;
block MappedCoefficients "MSL table-mapped aerodynamic coefficients versus speed"
  parameter Real coefficientTable[:, 4] = [
    0, 0.3, 0, 0;
    30, 0.3, 0, 0;
    60, 0.32, 0, -0.05]
    "Speed, drag coefficient, side coefficient, lift coefficient";
  ModelicaAutomotive.Interfaces.RealInput speed(unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput dragCoefficient;
  ModelicaAutomotive.Interfaces.RealOutput sideCoefficient;
  ModelicaAutomotive.Interfaces.RealOutput liftCoefficient;
protected
  Modelica.Blocks.Tables.CombiTable1Ds table(
    table=coefficientTable,
    columns={2, 3, 4},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=Modelica.Blocks.Types.Extrapolation.HoldLastPoint);
equation
  table.u = speed;
  dragCoefficient = table.y[1];
  sideCoefficient = table.y[2];
  liftCoefficient = table.y[3];
end MappedCoefficients;
