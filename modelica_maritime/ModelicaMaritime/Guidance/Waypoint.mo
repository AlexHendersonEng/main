within ModelicaMaritime.Guidance;
block Waypoint "Three-dimensional NED waypoint guidance"
  parameter ModelicaMaritime.Types.Length acceptanceRadius = 10;
  ModelicaMaritime.Interfaces.Vector3Input positionNED(each unit="m");
  ModelicaMaritime.Interfaces.Vector3Input waypointNED(each unit="m");
  ModelicaMaritime.Interfaces.RealOutput commandedHeading(unit="rad");
  ModelicaMaritime.Interfaces.RealOutput commandedDepth(unit="m");
  ModelicaMaritime.Interfaces.RealOutput horizontalDistance(unit="m");
  ModelicaMaritime.Interfaces.RealOutput distance(unit="m");
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
  commandedHeading = ModelicaMaritime.Mathematics.wrapHeading(
    atan2(displacement[2], displacement[1]));
  commandedDepth = waypointNED[3];
  reached = distance <= acceptanceRadius;
end Waypoint;
