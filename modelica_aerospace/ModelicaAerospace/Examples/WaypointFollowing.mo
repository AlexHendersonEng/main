within ModelicaAerospace.Examples;
model WaypointFollowing "Closed-loop horizontal waypoint following"
  parameter ModelicaAerospace.Types.Mass mass = 500;
  parameter ModelicaAerospace.Types.Velocity speed = 50;
  parameter ModelicaAerospace.Types.Length waypointNED[3] = {1000, 500, 0};
  ModelicaAerospace.FlightDynamics.PointMass.FlightPath vehicle(
    mass=mass,
    initialPositionNED={0, 0, 0},
    initialSpeed=speed,
    initialGroundTrack=0);
  ModelicaAerospace.Guidance.WaypointGuidance guidance(
    acceptanceRadius=25);
  output Real positionNED[3](each unit="m");
  output Real groundTrack(unit="rad");
  output Real commandedHeading(unit="rad");
  output Real distanceToWaypoint(unit="m");
  output Boolean waypointReached;
protected
  Real headingError;
  Real lateralAcceleration(unit="m/s2");
equation
  guidance.positionNED = vehicle.positionNED;
  guidance.waypointNED = waypointNED;
  headingError = ModelicaAerospace.Mathematics.wrapAngle(
    guidance.commandedHeading - vehicle.groundTrack);
  lateralAcceleration =
    ModelicaAerospace.Mathematics.clamp(1.2 * speed * headingError, -15, 15);
  vehicle.forceVelocityAxes = {
    0,
    mass * 9.80665,
    mass * lateralAcceleration};
  positionNED = vehicle.positionNED;
  groundTrack = vehicle.groundTrack;
  commandedHeading = guidance.commandedHeading;
  distanceToWaypoint = guidance.distance;
  waypointReached = guidance.reached;
  annotation (
    experiment(StartTime=0, StopTime=30, Tolerance=1e-8, Interval=0.05),
    Documentation(info="<html>
<p>Closes waypoint line-of-sight guidance around a constant-speed flight-path
plant. A bounded proportional turn command steers toward a waypoint 1 km
north and 0.5 km east; the 25 m acceptance sphere defines success.</p>
</html>"));
end WaypointFollowing;
