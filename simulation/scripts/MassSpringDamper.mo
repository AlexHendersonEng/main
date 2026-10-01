model MassSpringDamper
  Real x(start = 1, fixed = true);
  Real v(start = 0, fixed = true);
  parameter Real k = 1, c = 0.1, m = 1;
equation
  der(x) = v;
  m * der(v) = -k * x - c * v;
end MassSpringDamper;
