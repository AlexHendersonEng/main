within ModelicaMaritime.Examples;
model UnderwaterDepthHeadingHold "Closed-loop underwater depth, heading, and speed hold"
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
  output Real headingError;
  output Real depthError;
  output Real quaternionNorm;
protected
  Real measuredHeading(unit="rad");
equation
  vehicle.currentVelocityNED = {0.2, 0.1, 0};
  measuredHeading = ModelicaMaritime.Mathematics.wrapHeading(
    atan2(
      vehicle.rotationBodyToNED[2, 1],
      vehicle.rotationBodyToNED[1, 1]));
  headingController.commandedHeading = 0.5;
  headingController.heading = measuredHeading;
  depthController.setpoint = 30;
  depthController.measurement = vehicle.positionNED[3];
  speedController.setpoint = 1.5;
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
    1800 * headingController.command};
  positionNED = vehicle.positionNED;
  velocityBody = vehicle.velocityBody;
  headingError = headingController.headingError;
  depthError = 30 - vehicle.positionNED[3];
  quaternionNorm = vehicle.quaternionNorm;
  annotation (
    experiment(StartTime=0, StopTime=80, Tolerance=1e-8, Interval=0.1),
    Documentation(info="<html>
<p>A neutrally buoyant underwater vehicle holds positive-down depth, wrapped
heading, and through-water surge speed in a steady current using limited
controllers and a fixed propulsor.</p>
</html>"));
end UnderwaterDepthHeadingHold;
