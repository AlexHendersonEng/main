within ModelicaAerospace.Environment.Atmosphere;
function dynamicPressure "Calculate dynamic pressure"
  input ModelicaAerospace.Types.Density density;
  input ModelicaAerospace.Types.Velocity trueAirspeed;
  output ModelicaAerospace.Types.Pressure pressure;
algorithm
  assert(density >= 0, "Density must be non-negative");
  pressure := 0.5 * density * trueAirspeed * trueAirspeed;
end dynamicPressure;
