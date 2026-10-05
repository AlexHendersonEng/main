within ModelicaAutomotive;
package Constants "Automotive constants and stable array indices"
  constant Real standardGravity(unit="m/s2") = 9.80665;
  constant Real small = 1e-9 "Default numerical regularization threshold";
  constant Integer frontLeft = 1;
  constant Integer frontRight = 2;
  constant Integer rearLeft = 3;
  constant Integer rearRight = 4;
  annotation (Documentation(info="<html><p>Physical constants, numerical thresholds, and stable wheel-corner indices.</p></html>"));
end Constants;
