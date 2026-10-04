model TwoParam "y = b * exp(-a t); c is deliberately unused"
  parameter Real a = 1.0;
  parameter Real b = 2.0;
  parameter Real c = 1.0;
  Real x(start = 1.0, fixed = true);
  output Real y;
equation
  der(x) = -a * x;
  y = b * x;
end TwoParam;
