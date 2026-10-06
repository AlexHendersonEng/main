within ModelicaMaritime.Hydrodynamics;
block TablePlanarCoefficients "Interpolate planar coefficients against one abscissa"
  parameter Real tableX[:, 2] = [0, 0; 1, 0];
  parameter Real tableY[:, 2] = [0, 0; 1, 0];
  parameter Real tableN[:, 2] = [0, 0; 1, 0];
  parameter Modelica.Blocks.Types.Extrapolation extrapolation =
    Modelica.Blocks.Types.Extrapolation.HoldLastPoint;
  ModelicaMaritime.Interfaces.RealInput abscissa;
  ModelicaMaritime.Interfaces.Vector3Output coefficients "{CX, CY, CN}";
protected
  Modelica.Blocks.Tables.CombiTable1Ds xTable(
    table=tableX,
    columns={2},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=extrapolation);
  Modelica.Blocks.Tables.CombiTable1Ds yTable(
    table=tableY,
    columns={2},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=extrapolation);
  Modelica.Blocks.Tables.CombiTable1Ds nTable(
    table=tableN,
    columns={2},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=extrapolation);
equation
  xTable.u = abscissa;
  yTable.u = abscissa;
  nTable.u = abscissa;
  coefficients = {xTable.y[1], yTable.y[1], nTable.y[1]};
end TablePlanarCoefficients;
