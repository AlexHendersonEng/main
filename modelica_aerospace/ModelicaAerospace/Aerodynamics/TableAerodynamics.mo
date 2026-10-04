within ModelicaAerospace.Aerodynamics;
block TableAerodynamics "Angle-of-attack table with explicit extrapolation policy"
  parameter Integer pointCount(min=2) = 3;
  parameter Real angleTable[pointCount] = {-0.2, 0, 0.2};
  parameter Real liftTable[pointCount] = {-0.8, 0, 0.8};
  parameter Real dragTable[pointCount] = {0.12, 0.02, 0.12};
  parameter Real pitchingMomentTable[pointCount] = {0.1, 0, -0.1};
  parameter Modelica.Blocks.Types.Extrapolation extrapolation =
    Modelica.Blocks.Types.Extrapolation.HoldLastPoint;
  ModelicaAerospace.Interfaces.RealInput angleOfAttack(unit="rad");
  ModelicaAerospace.Interfaces.Vector3Output forceCoefficients;
  ModelicaAerospace.Interfaces.Vector3Output momentCoefficients;
protected
  Modelica.Blocks.Tables.CombiTable1Ds liftLookup(
    table=transpose({angleTable, liftTable}),
    columns={2},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=extrapolation);
  Modelica.Blocks.Tables.CombiTable1Ds dragLookup(
    table=transpose({angleTable, dragTable}),
    columns={2},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=extrapolation);
  Modelica.Blocks.Tables.CombiTable1Ds pitchingMomentLookup(
    table=transpose({angleTable, pitchingMomentTable}),
    columns={2},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=extrapolation);
  Real lift;
  Real drag;
  Real pitchingMoment;
equation
  liftLookup.u = angleOfAttack;
  dragLookup.u = angleOfAttack;
  pitchingMomentLookup.u = angleOfAttack;
  lift = liftLookup.y[1];
  drag = dragLookup.y[1];
  pitchingMoment = pitchingMomentLookup.y[1];
  forceCoefficients = {-drag, 0, -lift};
  momentCoefficients = {0, pitchingMoment, 0};
end TableAerodynamics;
