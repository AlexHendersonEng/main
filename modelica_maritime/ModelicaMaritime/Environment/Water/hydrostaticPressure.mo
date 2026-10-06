within ModelicaMaritime.Environment.Water;
function hydrostaticPressure "Absolute hydrostatic pressure at non-negative depth"
  input ModelicaMaritime.Types.Length depth "Depth below surface, positive down";
  input ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  input ModelicaMaritime.Types.Pressure surfacePressure =
    ModelicaMaritime.Constants.standardAtmosphericPressure;
  input Real gravity(unit="m/s2") = ModelicaMaritime.Constants.standardGravity;
  output ModelicaMaritime.Types.Pressure pressure;
algorithm
  assert(depth >= 0, "Hydrostatic pressure requires non-negative depth");
  assert(density > 0, "Water density must be positive");
  assert(gravity > 0, "Gravity must be positive");
  pressure := surfacePressure + density * gravity * depth;
end hydrostaticPressure;
