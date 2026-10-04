within ModelicaAerospace.Environment.Wind;
function drydenParameters "Calculate MIL-F-8785C low-altitude Dryden scales"
  input ModelicaAerospace.Types.Length altitude;
  input ModelicaAerospace.Types.Velocity windSpeedAt20Feet;
  output ModelicaAerospace.Types.Length lengthScale[3];
  output ModelicaAerospace.Types.Velocity sigma[3];
protected
  Real altitudeFeet;
  Real denominator;
algorithm
  assert(altitude >= 0 and altitude <= 304.8, "Low-altitude Dryden model requires 0 to 304.8 m");
  assert(windSpeedAt20Feet >= 0, "Wind speed at 20 feet must be non-negative");
  altitudeFeet := max(altitude / 0.3048, 1);
  denominator := 0.177 + 0.000823 * altitudeFeet;
  lengthScale := {
    altitudeFeet / denominator ^ 1.2,
    altitudeFeet,
    altitudeFeet} * 0.3048;
  sigma[3] := 0.1 * windSpeedAt20Feet;
  sigma[1] := sigma[3] / denominator ^ 0.4;
  sigma[2] := sigma[1];
end drydenParameters;
