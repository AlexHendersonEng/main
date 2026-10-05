within ModelicaAerospace.Guidance;
block LinePathGuidance "Look-ahead guidance toward a horizontal NED line"
  parameter ModelicaAerospace.Types.Length lookAheadDistance = 100;
  ModelicaAerospace.Interfaces.Vector3Input positionNED(each unit="m");
  ModelicaAerospace.Interfaces.Vector3Input pathStartNED(each unit="m");
  ModelicaAerospace.Interfaces.Vector3Input pathEndNED(each unit="m");
  ModelicaAerospace.Interfaces.RealOutput pathHeading(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput commandedHeading(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput crossTrackError(unit="m");
  ModelicaAerospace.Interfaces.RealOutput alongTrackDistance(unit="m");
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
  assert(pathLength > 1e-12, "Guidance path must have nonzero horizontal length");
  pathHeading = atan2(pathEast, pathNorth);
  alongTrackDistance =
    (relativeNorth * pathNorth + relativeEast * pathEast) / pathLength;
  crossTrackError =
    (pathNorth * relativeEast - pathEast * relativeNorth) / pathLength;
  commandedHeading = ModelicaAerospace.Mathematics.wrapAngle(
    pathHeading - atan2(crossTrackError, lookAheadDistance));
end LinePathGuidance;
