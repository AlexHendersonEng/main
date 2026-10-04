within ModelicaAerospace.Constants;
package Numerics "Shared numerical tolerances"
  constant Real small = 1e-12 "Guard against division by zero";
  constant Real quaternionNormTolerance = 1e-10;
  constant Real rotationOrthogonalityTolerance = 1e-10;
  annotation (Documentation(info="<html><p>Default numerical tolerances.</p></html>"));
end Numerics;
