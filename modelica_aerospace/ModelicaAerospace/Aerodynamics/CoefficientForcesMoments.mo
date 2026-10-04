within ModelicaAerospace.Aerodynamics;
block CoefficientForcesMoments "Scale body-axis coefficients into forces and moments"
  parameter ModelicaAerospace.Types.Length referenceArea = 1;
  parameter ModelicaAerospace.Types.Length referenceSpan = 1;
  parameter ModelicaAerospace.Types.Length referenceChord = 1;
  ModelicaAerospace.Interfaces.RealInput dynamicPressure(unit="Pa");
  ModelicaAerospace.Interfaces.Vector3Input forceCoefficients
    "{CX, CY, CZ}";
  ModelicaAerospace.Interfaces.Vector3Input momentCoefficients
    "{Cl, Cm, Cn}";
  ModelicaAerospace.Interfaces.Vector3Output forceBody(each unit="N");
  ModelicaAerospace.Interfaces.Vector3Output momentBody(each unit="N.m");
equation
  assert(referenceArea > 0, "Reference area must be positive");
  assert(referenceSpan > 0, "Reference span must be positive");
  assert(referenceChord > 0, "Reference chord must be positive");
  forceBody = dynamicPressure * referenceArea * forceCoefficients;
  momentBody = dynamicPressure * referenceArea * {
    referenceSpan * momentCoefficients[1],
    referenceChord * momentCoefficients[2],
    referenceSpan * momentCoefficients[3]};
end CoefficientForcesMoments;
