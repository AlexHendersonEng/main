within ModelicaMaritime.Guidance;
block LineOfSight "Look-ahead guidance toward a horizontal NED line"
  parameter ModelicaMaritime.Types.Length lookAheadDistance = 100;
  ModelicaMaritime.Interfaces.Vector3Input positionNED(each unit="m");
  ModelicaMaritime.Interfaces.Vector3Input pathStartNED(each unit="m");
  ModelicaMaritime.Interfaces.Vector3Input pathEndNED(each unit="m");
  ModelicaMaritime.Interfaces.RealOutput pathHeading(unit="rad");
  ModelicaMaritime.Interfaces.RealOutput commandedHeading(unit="rad");
  ModelicaMaritime.Interfaces.RealOutput crossTrackError(unit="m");
  ModelicaMaritime.Interfaces.RealOutput alongTrackDistance(unit="m");
protected
  Real pathNorth;
  Real pathEast;
  Real relativeNorth;
  Real relativeEast;
  Real pathLength;
equation
  assert(lookAheadDistance > 0, "Look-ahead distance must be positive");
  pathNorth = pathEndNED[1] - pathStartNED[1];
  pathEast = pathEndNED[2] - pathStartNED[2];
  relativeNorth = positionNED[1] - pathStartNED[1];
  relativeEast = positionNED[2] - pathStartNED[2];
  pathLength = sqrt(pathNorth * pathNorth + pathEast * pathEast);
  assert(pathLength > 1e-12, "Guidance path must be nonzero");
  pathHeading = ModelicaMaritime.Mathematics.wrapHeading(atan2(pathEast, pathNorth));
  alongTrackDistance =
    (relativeNorth * pathNorth + relativeEast * pathEast) / pathLength;
  crossTrackError =
    (pathNorth * relativeEast - pathEast * relativeNorth) / pathLength;
  commandedHeading = ModelicaMaritime.Mathematics.wrapHeading(
    pathHeading - atan2(crossTrackError, lookAheadDistance));
end LineOfSight;
