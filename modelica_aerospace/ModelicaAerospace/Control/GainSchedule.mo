within ModelicaAerospace.Control;
block GainSchedule "MSL table-based proportional, integral, and derivative gains"
  parameter Integer pointCount(min=2) = 3;
  parameter Real schedulingPoints[pointCount] = {0, 1, 2};
  parameter Real proportionalGain[pointCount] = {1, 1, 1};
  parameter Real integralGain[pointCount] = {0, 0, 0};
  parameter Real derivativeGain[pointCount] = {0, 0, 0};
  parameter Modelica.Blocks.Types.Extrapolation extrapolation =
    Modelica.Blocks.Types.Extrapolation.HoldLastPoint;
  ModelicaAerospace.Interfaces.RealInput schedulingVariable;
  ModelicaAerospace.Interfaces.Vector3Output gains
    "{proportional, integral, derivative}";
protected
  Modelica.Blocks.Tables.CombiTable1Ds table(
    table=transpose({
      schedulingPoints,
      proportionalGain,
      integralGain,
      derivativeGain}),
    columns={2, 3, 4},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=extrapolation);
equation
  table.u = schedulingVariable;
  gains = table.y;
end GainSchedule;
