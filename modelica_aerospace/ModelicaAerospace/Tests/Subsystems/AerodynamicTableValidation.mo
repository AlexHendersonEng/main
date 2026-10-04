within ModelicaAerospace.Tests.Subsystems;
model AerodynamicTableValidation "Exercise MSL aerodynamic interpolation and extrapolation"
  ModelicaAerospace.Aerodynamics.TableAerodynamics clampedTable(
    pointCount=3,
    angleTable={-0.2, 0, 0.2},
    liftTable={-0.8, 0, 0.8},
    dragTable={0.12, 0.02, 0.12},
    pitchingMomentTable={0.1, 0, -0.1},
    extrapolation=Modelica.Blocks.Types.Extrapolation.HoldLastPoint);
  ModelicaAerospace.Aerodynamics.TableAerodynamics linearTable(
    pointCount=3,
    angleTable={-0.2, 0, 0.2},
    liftTable={-0.8, 0, 0.8},
    dragTable={0.12, 0.02, 0.12},
    pitchingMomentTable={0.1, 0, -0.1},
    extrapolation=Modelica.Blocks.Types.Extrapolation.LastTwoPoints);
  Modelica.Blocks.Tables.CombiTable1Ds interpolationTable(
    table=[-0.2, -1; 0, 0; 0.2, 2],
    columns={2},
    smoothness=Modelica.Blocks.Types.Smoothness.LinearSegments,
    extrapolation=Modelica.Blocks.Types.Extrapolation.HoldLastPoint);
  output Real clampedLift;
  output Real clampedDrag;
  output Real clampedPitchingMoment;
  output Real linearLift;
  output Real linearDrag;
  output Real linearPitchingMoment;
  output Real interpolatedValue;
protected
  Real probe;
equation
  probe = 1e-6 * time;
  clampedTable.angleOfAttack = 0.3;
  linearTable.angleOfAttack = 0.3;
  clampedLift = -clampedTable.forceCoefficients[3] + probe;
  clampedDrag = -clampedTable.forceCoefficients[1] + probe;
  clampedPitchingMoment = clampedTable.momentCoefficients[2] + probe;
  linearLift = -linearTable.forceCoefficients[3] + probe;
  linearDrag = -linearTable.forceCoefficients[1] + probe;
  linearPitchingMoment = linearTable.momentCoefficients[2] + probe;
  interpolationTable.u = 0.1;
  interpolatedValue = interpolationTable.y[1] + probe;
end AerodynamicTableValidation;
