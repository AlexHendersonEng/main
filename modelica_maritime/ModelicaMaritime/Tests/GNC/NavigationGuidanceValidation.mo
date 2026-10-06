within ModelicaMaritime.Tests.GNC;
model NavigationGuidanceValidation "Validate marine navigation and guidance geometry"
  ModelicaMaritime.Navigation.KinematicOutputs kinematics;
  ModelicaMaritime.Navigation.DeadReckoning deadReckoning(
    initialPositionNED={1, 2, 3},
    initialHeading=6.2);
  ModelicaMaritime.Navigation.ComplementaryFilter headingFilter(
    bandwidth=2,
    initialEstimate=6.2,
    angularState=true);
  ModelicaMaritime.Navigation.PositionFusion positionFusion(
    bandwidth=1,
    initialPositionNED={0, 0, 0});
  ModelicaMaritime.Guidance.LineOfSight line(lookAheadDistance=20);
  ModelicaMaritime.Guidance.Waypoint waypoint(acceptanceRadius=2);
  ModelicaMaritime.Guidance.DepthAltitude depthGuidance(altitudeMode=false);
  ModelicaMaritime.Guidance.DepthAltitude altitudeGuidance(altitudeMode=true);
  ModelicaMaritime.Guidance.CurrentCompensatedHeading currentCompensation;
  ModelicaMaritime.Guidance.CommandErrors errors;
  output Real speed;
  output Real horizontalSpeed;
  output Real groundTrack;
  output Real depthRate;
  output Real sideslip;
  output Real deadReckonedPosition[3];
  output Real deadReckonedHeading;
  output Real filteredHeading;
  output Real fusedPosition[3];
  output Real pathHeading;
  output Real commandedLineHeading;
  output Real crossTrackError;
  output Real alongTrackDistance;
  output Real waypointHeading;
  output Real waypointDepth;
  output Real waypointDistance;
  output Real depthCommand;
  output Real altitudeDepthCommand;
  output Real compensatedHeading;
  output Real requiredWaterSpeed;
  output Real headingError;
  output Real depthError;
  output Real speedError;
equation
  kinematics.velocityNED = {3, 4, 1};
  kinematics.velocityBody = {4, 1, 0};
  deadReckoning.velocityNED = {1, 2, 0.5};
  deadReckoning.yawRate = 0.1;
  headingFilter.rate = 0.1;
  headingFilter.absoluteMeasurement = 0.2;
  positionFusion.velocityNED = {1, 0, 0};
  positionFusion.absolutePositionNED = {10, 5, 2};
  line.positionNED = {20, 10, 0};
  line.pathStartNED = {0, 0, 0};
  line.pathEndNED = {100, 0, 0};
  waypoint.positionNED = {10, 20, 5};
  waypoint.waypointNED = {13, 24, 17};
  depthGuidance.requestedDepth = 30;
  depthGuidance.requestedAltitude = 20;
  depthGuidance.seafloorDepth = 100;
  altitudeGuidance.requestedDepth = 30;
  altitudeGuidance.requestedAltitude = 20;
  altitudeGuidance.seafloorDepth = 100;
  currentCompensation.desiredGroundTrack = 0;
  currentCompensation.desiredGroundSpeed = 2;
  currentCompensation.currentVelocityNED = {0, 0.5, 0};
  errors.commandedHeading = 0.1;
  errors.heading = 6.2;
  errors.commandedDepth = 30;
  errors.depth = 25;
  errors.commandedSpeed = 4;
  errors.speed = 3;
  speed = kinematics.speed;
  horizontalSpeed = kinematics.horizontalSpeed;
  groundTrack = kinematics.groundTrack;
  depthRate = kinematics.depthRate;
  sideslip = kinematics.sideslip;
  deadReckonedPosition = deadReckoning.estimatedPositionNED;
  deadReckonedHeading = deadReckoning.estimatedHeading;
  filteredHeading = headingFilter.estimate;
  fusedPosition = positionFusion.estimatedPositionNED;
  pathHeading = line.pathHeading;
  commandedLineHeading = line.commandedHeading;
  crossTrackError = line.crossTrackError;
  alongTrackDistance = line.alongTrackDistance;
  waypointHeading = waypoint.commandedHeading;
  waypointDepth = waypoint.commandedDepth;
  waypointDistance = waypoint.distance;
  depthCommand = depthGuidance.commandedDepth;
  altitudeDepthCommand = altitudeGuidance.commandedDepth;
  compensatedHeading = currentCompensation.commandedHeading;
  requiredWaterSpeed = currentCompensation.requiredWaterSpeed;
  headingError = errors.headingError;
  depthError = errors.depthError;
  speedError = errors.speedError;
end NavigationGuidanceValidation;
