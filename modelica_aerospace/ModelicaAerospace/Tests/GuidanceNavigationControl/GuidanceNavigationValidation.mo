within ModelicaAerospace.Tests.GuidanceNavigationControl;
model GuidanceNavigationValidation "Exercise guidance geometry and navigation outputs"
  ModelicaAerospace.Guidance.WaypointGuidance waypoint(acceptanceRadius=5);
  ModelicaAerospace.Guidance.WaypointGuidance reachedWaypoint(acceptanceRadius=5);
  ModelicaAerospace.Guidance.LinePathGuidance line(lookAheadDistance=50);
  ModelicaAerospace.Guidance.CommandErrors errors;
  ModelicaAerospace.Navigation.KinematicOutputs kinematics;
  output Real waypointHeading;
  output Real waypointFlightPathAngle;
  output Real waypointHorizontalDistance;
  output Real waypointDistance;
  output Real waypointReached;
  output Real coincidentWaypointReached;
  output Real pathHeading;
  output Real commandedPathHeading;
  output Real crossTrackError;
  output Real alongTrackDistance;
  output Real headingError;
  output Real altitudeError;
  output Real speedError;
  output Real speed;
  output Real groundSpeed;
  output Real groundTrack;
  output Real flightPathAngle;
equation
  waypoint.positionNED = {0, 0, 0};
  waypoint.waypointNED = {100, 100, -100};
  waypointHeading = waypoint.commandedHeading;
  waypointFlightPathAngle = waypoint.commandedFlightPathAngle;
  waypointHorizontalDistance = waypoint.horizontalDistance;
  waypointDistance = waypoint.distance;
  waypointReached = if waypoint.reached then 1 else 0;

  reachedWaypoint.positionNED = {4, -2, 1};
  reachedWaypoint.waypointNED = {4, -2, 1};
  coincidentWaypointReached = if reachedWaypoint.reached then 1 else 0;

  line.positionNED = {20, 10, 0};
  line.pathStartNED = {0, 0, 0};
  line.pathEndNED = {100, 0, 0};
  pathHeading = line.pathHeading;
  commandedPathHeading = line.commandedHeading;
  crossTrackError = line.crossTrackError;
  alongTrackDistance = line.alongTrackDistance;

  errors.commandedHeading = -179 * 3.141592653589793 / 180;
  errors.heading = 179 * 3.141592653589793 / 180;
  errors.commandedAltitude = 1000;
  errors.altitude = 900;
  errors.commandedSpeed = 120;
  errors.speed = 100;
  headingError = errors.headingError;
  altitudeError = errors.altitudeError;
  speedError = errors.speedError;

  kinematics.velocityNED = {100, 100, -20};
  speed = kinematics.speed;
  groundSpeed = kinematics.groundSpeed;
  groundTrack = kinematics.groundTrack;
  flightPathAngle = kinematics.flightPathAngle;
end GuidanceNavigationValidation;
