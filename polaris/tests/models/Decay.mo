model Decay
  parameter Real k = 1.0;
  Real x(start = 1.0, fixed = true);
  input Real u = 0;
  output Real y;
equation
  der(x) = -k * x + u;
  y = 2 * x;
end Decay;
