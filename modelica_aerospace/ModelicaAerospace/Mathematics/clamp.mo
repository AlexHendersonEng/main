within ModelicaAerospace.Mathematics;
function clamp "Limit a scalar to inclusive bounds"
  input Real value;
  input Real minimum;
  input Real maximum;
  output Real limited;
algorithm
  assert(minimum <= maximum, "minimum must not exceed maximum");
  limited := if value < minimum then minimum else if value > maximum then maximum else value;
end clamp;
