model Driven "First-order lag driven by an input u"
  parameter Real a = 2.0 "Pole [1/s]";
  parameter Real gain = 1.0;
  input Real u(start = 0.0) "Driving input";
  Real x(start = 0.0, fixed = true);
equation
  der(x) = -a * x + gain * u;
end Driven;
