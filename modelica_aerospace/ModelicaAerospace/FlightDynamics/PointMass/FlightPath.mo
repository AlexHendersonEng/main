within ModelicaAerospace.FlightDynamics.PointMass;
block FlightPath "Flight-path-coordinate three-degree-of-freedom point mass"
  parameter ModelicaAerospace.Types.Mass mass = 1;
  parameter ModelicaAerospace.Types.Acceleration gravity = 9.80665;
  parameter ModelicaAerospace.Types.Length initialPositionNED[3] = {0, 0, 0};
  parameter ModelicaAerospace.Types.Velocity initialSpeed = 100;
  parameter ModelicaAerospace.Types.Angle initialFlightPathAngle = 0;
  parameter ModelicaAerospace.Types.Angle initialGroundTrack = 0;
  ModelicaAerospace.Interfaces.Vector3Input forceVelocityAxes(each unit="N")
    "{tangential, upward-normal, right-lateral}";
  ModelicaAerospace.Interfaces.Vector3Output positionNED(each unit="m");
  ModelicaAerospace.Interfaces.Vector3Output velocityNED(each unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput speed(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput flightPathAngle(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput groundTrack(unit="rad");
protected
  Real positionState[3](start=initialPositionNED, each fixed=true);
  Real speedState(start=initialSpeed, fixed=true);
  Real flightPathState(start=initialFlightPathAngle, fixed=true);
  Real trackState(start=initialGroundTrack, fixed=true);
equation
  assert(mass > 0, "Point-mass mass must be positive");
  assert(speedState > 1e-12, "Speed must remain positive");
  assert(
    abs(cos(flightPathState)) > 1e-8,
    "Flight-path coordinates are singular for vertical flight");
  velocityNED = {
    speedState * cos(flightPathState) * cos(trackState),
    speedState * cos(flightPathState) * sin(trackState),
    -speedState * sin(flightPathState)};
  der(positionState) = velocityNED;
  der(speedState) = forceVelocityAxes[1] / mass - gravity * sin(flightPathState);
  der(flightPathState) = (
    forceVelocityAxes[2] / mass - gravity * cos(flightPathState)) / speedState;
  der(trackState) =
    forceVelocityAxes[3] / (mass * speedState * cos(flightPathState));
  positionNED = positionState;
  speed = speedState;
  flightPathAngle = flightPathState;
  groundTrack = trackState;
end FlightPath;
