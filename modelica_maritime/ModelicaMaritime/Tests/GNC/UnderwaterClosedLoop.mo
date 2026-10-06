within ModelicaMaritime.Tests.GNC;
model UnderwaterClosedLoop "Six-DoF underwater waypoint, depth, heading, and speed control"
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF vehicle(
    massProperties(
      mass=500,
      centerOfGravityBody={0, 0, 0.2},
      centerOfBuoyancyBody={0, 0, -0.2},
      inertiaBody=[300, 0, 0; 0, 400, 0; 0, 0, 500],
      displacedVolume=500 / 1025),
    hydrodynamics(
      addedMass=diagonal({100, 150, 180, 50, 60, 80}),
      linearDamping=diagonal({100, 200, 250, 100, 120, 180}),
      quadraticDamping=diagonal({40, 80, 100, 20, 25, 40})),
    initialState(
      positionNED={0, 0, 10},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}));
  ModelicaMaritime.Guidance.Waypoint guidance(acceptanceRadius=5);
  ModelicaMaritime.Control.HeadingController headingController(
    k=1.2,
    Ti=10,
    outputMaximum=1,
    outputMinimum=-1);
  ModelicaMaritime.Control.LimitedPID depthController(
    k=0.08,
    Ti=15,
    outputMaximum=1,
    outputMinimum=-1);
  ModelicaMaritime.Control.LimitedPID speedController(
    k=0.8,
    Ti=6,
    outputMaximum=1,
    outputMinimum=0);
  ModelicaMaritime.Propulsion.FixedThruster propulsor(
    maximumForwardThrust=800,
    maximumReverseThrust=300);
  output Real positionNED[3];
  output Real velocityBody[6];
  output Real distance;
  output Real headingError;
  output Real depthError;
  output Real quaternionNorm;
protected
  Real measuredHeading(unit="rad");
  discrete Boolean waypointCaptured(start=false, fixed=true);
equation
  when guidance.reached then
    waypointCaptured = true;
  end when;
  vehicle.currentVelocityNED = {0.2, 0.1, 0};
  guidance.positionNED = vehicle.positionNED;
  guidance.waypointNED = {100, 20, 30};
  measuredHeading = ModelicaMaritime.Mathematics.wrapHeading(
    atan2(
      vehicle.rotationBodyToNED[2, 1],
      vehicle.rotationBodyToNED[1, 1]));
  headingController.commandedHeading = if waypointCaptured then
    measuredHeading else guidance.commandedHeading;
  headingController.heading = measuredHeading;
  depthController.setpoint = guidance.commandedDepth;
  depthController.measurement = vehicle.positionNED[3];
  speedController.setpoint = if waypointCaptured then 0 else 2;
  speedController.measurement = vehicle.relativeVelocityBody[1];
  propulsor.command = speedController.command;
  propulsor.enabled = true;
  propulsor.failed = false;
  vehicle.generalizedForceBody = {
    propulsor.generalizedLoadBody[1],
    0,
    1200 * depthController.command,
    0,
    0,
    if waypointCaptured then 0 else 1800 * headingController.command};
  positionNED = vehicle.positionNED;
  velocityBody = vehicle.velocityBody;
  distance = guidance.distance;
  headingError = headingController.headingError;
  depthError = guidance.commandedDepth - vehicle.positionNED[3];
  quaternionNorm = vehicle.quaternionNorm;
end UnderwaterClosedLoop;
