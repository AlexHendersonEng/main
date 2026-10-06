within ModelicaMaritime.Examples;
model PropulsionFailureResponse "Twin-thruster surface path response after port-thruster failure"
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
      positionNED={0, 10},
      heading=0,
      velocityBody={0, 0},
      yawRate=0));
  ModelicaMaritime.Guidance.LineOfSight guidance(lookAheadDistance=30);
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
  ModelicaMaritime.Control.PlanarAllocator allocator(differentialYawGain=0.8);
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
  output Real headingError;
  output Boolean portFailed;
equation
  vessel.currentVelocityNED = {0, 0.3, 0};
  guidance.positionNED = {vessel.poseNED[1], vessel.poseNED[2], 0};
  guidance.pathStartNED = {0, 0, 0};
  guidance.pathEndNED = {200, 0, 0};
  headingController.commandedHeading = guidance.commandedHeading;
  headingController.heading = vessel.poseNED[3];
  speedController.setpoint = 2;
  speedController.measurement = sqrt(
    vessel.relativeVelocityBody[1] * vessel.relativeVelocityBody[1]
    + vessel.relativeVelocityBody[2] * vessel.relativeVelocityBody[2]);
  allocator.surgeCommand = speedController.command;
  allocator.yawCommand = headingController.command;
  port.command = allocator.portThrusterCommand;
  port.enabled = true;
  port.failed = time >= 40;
  starboard.command = allocator.starboardThrusterCommand;
  starboard.enabled = true;
  starboard.failed = false;
  vessel.generalizedForceBody = {
    port.generalizedLoadBody[1] + starboard.generalizedLoadBody[1],
    0,
    port.generalizedLoadBody[6] + starboard.generalizedLoadBody[6]};
  poseNED = vessel.poseNED;
  velocityBody = vessel.velocityBody;
  crossTrackError = guidance.crossTrackError;
  headingError = headingController.headingError;
  portFailed = port.failed;
  annotation (
    experiment(StartTime=0, StopTime=80, Tolerance=1e-8, Interval=0.1),
    Documentation(info="<html>
<p>A current-disturbed surface vessel follows a northbound line with twin
thrusters until the port unit fails at 40 s. The remaining allocation and
hydrodynamic damping produce a bounded degraded response.</p>
</html>"));
end PropulsionFailureResponse;
