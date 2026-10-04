within ModelicaAerospace.Environment.Gravity;
function constantGravityNED "Return constant standard gravity in NED coordinates"
  input ModelicaAerospace.Types.Acceleration magnitude =
    ModelicaAerospace.Constants.StandardAtmosphere.standardGravity;
  output ModelicaAerospace.Types.Acceleration gravityNED[3];
algorithm
  assert(magnitude >= 0, "Gravity magnitude must be non-negative");
  gravityNED := {0, 0, magnitude};
end constantGravityNED;
