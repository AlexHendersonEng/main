within ModelicaAutomotive.Mathematics;
function regularizedSign "Smooth sign approximation with a finite zero-speed slope"
  input Real value;
  input Real threshold(min=0) = 0.01;
  output Real y;
algorithm
  assert(threshold > 0, "regularizedSign threshold must be positive");
  y := value / sqrt(value * value + threshold * threshold);
end regularizedSign;
