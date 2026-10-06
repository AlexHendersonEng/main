within ModelicaAutomotive.Types;
record AerodynamicParameters "Automotive aerodynamic reference parameters"
  Real referenceArea(unit="m2", min=0) = 2.2;
  ModelicaAutomotive.Types.Length referenceLength = 2.7;
  Real dragCoefficient(min=0) = 0.3;
  Real sideForceDerivative(unit="1/rad") = 0;
  Real liftCoefficient = 0;
  Real momentCoefficients[3] = {0, 0, 0}
    "Roll, pitch, and yaw moment coefficients";
  ModelicaAutomotive.Types.Length applicationPointBody[3] = {0, 0, 0};
end AerodynamicParameters;
