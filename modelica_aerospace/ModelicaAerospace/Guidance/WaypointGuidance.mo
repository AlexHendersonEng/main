within ModelicaAerospace.Guidance;
block WaypointGuidance "Line-of-sight commands from current position to a waypoint"
  parameter ModelicaAerospace.Types.Length acceptanceRadius = 10;
  ModelicaAerospace.Interfaces.Vector3Input positionNED(each unit="m");
  ModelicaAerospace.Interfaces.Vector3Input waypointNED(each unit="m");
  ModelicaAerospace.Interfaces.RealOutput commandedHeading(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput commandedFlightPathAngle(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput horizontalDistance(unit="m");
  ModelicaAerospace.Interfaces.RealOutput distance(unit="m");
  output Boolean reached;
protected
  Real displacement[3];
equation
  assert(acceptanceRadius >= 0, "Waypoint acceptance radius must be non-negative");
  displacement = waypointNED - positionNED;
  horizontalDistance = sqrt(
    displacement[1] * displacement[1] + displacement[2] * displacement[2]);
  distance = sqrt(
    horizontalDistance * horizontalDistance + displacement[3] * displacement[3]);
  commandedHeading = atan2(displacement[2], displacement[1]);
  commandedFlightPathAngle = atan2(-displacement[3], horizontalDistance);
  reached = distance <= acceptanceRadius;
end WaypointGuidance;
