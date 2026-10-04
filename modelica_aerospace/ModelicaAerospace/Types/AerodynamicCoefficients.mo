within ModelicaAerospace.Types;
record AerodynamicCoefficients "Dimensionless body-axis aerodynamic coefficients"
  Real force[3] = {0, 0, 0} "{CX, CY, CZ}";
  Real moment[3] = {0, 0, 0} "{Cl, Cm, Cn}";
end AerodynamicCoefficients;
