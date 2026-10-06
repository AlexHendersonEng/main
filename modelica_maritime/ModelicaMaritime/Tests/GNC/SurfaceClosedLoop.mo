within ModelicaMaritime.Tests.GNC;
model SurfaceClosedLoop "LOS and current-compensated twin-thruster surface tracking"
  ModelicaMaritime.VesselDynamics.Generic.Planar3DOF vessel(
    massProperties(
      mass=500,
      centerOfGravityX=0,
      yawInertia=1000),
    hydrodynamics(
      addedMass=diagonal({100, 200, 300}),
      linearDamping=diagonal({80, 250, 500}),
      quadraticDamping=diagonal({20, 100, 150})),
    initialState(
      positionNED={0, 20},
      heading=0,
      velocityBody={0, 0},
      yawRate=0));
  ModelicaMaritime.Guidance.LineOfSight guidance(lookAheadDistance=30);
  ModelicaMaritime.Guidance.CurrentCompensatedHeading compensation;
  ModelicaMaritime.Control.HeadingController headingController(
    k=1.5,
    Ti=8,
    outputMaximum=1,
    outputMinimum=-1);
  ModelicaMaritime.Control.LimitedPID speedController(
    k=0.8,
    Ti=5,
    outputMaximum=1,
    outputMinimum=0);
  ModelicaMaritime.Control.PlanarAllocator allocator(
    differentialYawGain=0.8,
    rudderYawGain=1);
  ModelicaMaritime.Propulsion.FixedThruster port(
    maximumForwardThrust=400,
    maximumReverseThrust=200,
    applicationPointBody={0, -2, 0});
  ModelicaMaritime.Propulsion.FixedThruster starboard(
    maximumForwardThrust=400,
    maximumReverseThrust=200,
    applicationPointBody={0, 2, 0});
  output Real poseNED[3];
  output Real velocityBody[3];
  output Real crossTrackError;
  output Real commandedHeading;
  output Real headingError;
  output Real speedCommand;
equation
  vessel.currentVelocityNED = {0, 0.5, 0};
  guidance.positionNED = {vessel.poseNED[1], vessel.poseNED[2], 0};
  guidance.pathStartNED = {0, 0, 0};
  guidance.pathEndNED = {200, 0, 0};
  compensation.desiredGroundTrack = guidance.commandedHeading;
  compensation.desiredGroundSpeed = 2;
  compensation.currentVelocityNED = {0, 0.5, 0};
  headingController.commandedHeading = compensation.commandedHeading;
  headingController.heading = vessel.poseNED[3];
  speedController.setpoint = compensation.requiredWaterSpeed;
  speedController.measurement = sqrt(
    vessel.relativeVelocityBody[1] * vessel.relativeVelocityBody[1]
    + vessel.relativeVelocityBody[2] * vessel.relativeVelocityBody[2]);
  allocator.surgeCommand = speedController.command;
  allocator.yawCommand = headingController.command;
  port.command = allocator.portThrusterCommand;
  port.enabled = true;
  port.failed = false;
  starboard.command = allocator.starboardThrusterCommand;
  starboard.enabled = true;
  starboard.failed = false;
  vessel.generalizedForceBody = {
    port.generalizedLoadBody[1] + starboard.generalizedLoadBody[1],
    port.generalizedLoadBody[2] + starboard.generalizedLoadBody[2],
    port.generalizedLoadBody[6] + starboard.generalizedLoadBody[6]};
  poseNED = vessel.poseNED;
  velocityBody = vessel.velocityBody;
  crossTrackError = guidance.crossTrackError;
  commandedHeading = compensation.commandedHeading;
  headingError = headingController.headingError;
  speedCommand = speedController.command;
end SurfaceClosedLoop;
