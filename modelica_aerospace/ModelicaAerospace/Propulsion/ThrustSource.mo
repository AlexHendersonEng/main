within ModelicaAerospace.Propulsion;
block ThrustSource "Commanded fixed-direction thrust source"
  parameter ModelicaAerospace.Types.Force maximumThrust = 1;
  parameter Real directionBody[3] = {1, 0, 0};
  ModelicaAerospace.Interfaces.RealInput throttle;
  input Boolean enabled;
  ModelicaAerospace.Interfaces.Vector3Output forceBody(each unit="N");
  ModelicaAerospace.Interfaces.RealOutput thrust(unit="N");
protected
  Real directionNorm;
equation
  directionNorm = sqrt(
    directionBody[1] * directionBody[1]
    + directionBody[2] * directionBody[2]
    + directionBody[3] * directionBody[3]);
  assert(maximumThrust >= 0, "Maximum thrust must be non-negative");
  assert(directionNorm > 1e-12, "Thrust direction must be nonzero");
  thrust = if enabled then
    maximumThrust * ModelicaAerospace.Mathematics.clamp(throttle, 0, 1)
    else 0;
  forceBody = thrust * directionBody / directionNorm;
end ThrustSource;
