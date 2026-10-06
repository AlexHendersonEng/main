within ModelicaMaritime.Tests.Subsystems;
model LoadIntegrationValidation "Apply a common propulsor load to planar and 6-DoF dynamics"
  ModelicaMaritime.Propulsion.FixedThruster thruster(
    maximumForwardThrust=60,
    directionBody={1, 0, 0});
  ModelicaMaritime.VesselDynamics.Generic.Planar3DOF planar(
    massProperties(
      mass=10,
      centerOfGravityX=0,
      yawInertia=20),
    hydrodynamics(
      addedMass=zeros(3, 3),
      linearDamping=zeros(3, 3),
      quadraticDamping=zeros(3, 3)),
    initialState(
      positionNED={0, 0},
      heading=0,
      velocityBody={0, 0},
      yawRate=0));
  ModelicaMaritime.Propulsion.FirstOrderShaft shaft(
    timeConstant=0.5,
    maximumForwardRate=10);
  ModelicaMaritime.Actuators.Servo rudderServo(
    minimumPosition=-0.5,
    maximumPosition=0.5,
    risingRate=0.2,
    fallingRate=-0.2);
  ModelicaMaritime.VesselDynamics.MMG.Planar3DOF mmg(
    massProperties(
      mass=10,
      centerOfGravityX=0,
      yawInertia=20),
    hydrodynamics(
      addedMass=zeros(3, 3),
      linearDamping=zeros(3, 3),
      quadraticDamping=zeros(3, 3)),
    initialState(
      positionNED={0, 0},
      heading=0,
      velocityBody={0, 0},
      yawRate=0),
    density=1025,
    referenceLength=10,
    referenceDraft=2,
    hullCoefficients(),
    propellerProperties(thrustCoefficient={0, 0, 0}),
    rudderProperties(area=1, liftGradient=0));
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF spatial(
    massProperties(
      mass=10,
      inertiaBody=[2, 0, 0; 0, 3, 0; 0, 0, 4],
      displacedVolume=0),
    hydrodynamics(
      addedMass=zeros(6, 6),
      linearDamping=zeros(6, 6),
      quadraticDamping=zeros(6, 6)),
    initialState(
      positionNED={0, 0, 0},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    gravity=0);
  output Real planarPosition;
  output Real planarVelocity;
  output Real mmgPosition;
  output Real mmgVelocity;
  output Real spatialPosition;
  output Real spatialVelocity;
equation
  thruster.command = 0.5;
  thruster.enabled = true;
  thruster.failed = false;
  planar.generalizedForceBody = {
    thruster.generalizedLoadBody[1],
    thruster.generalizedLoadBody[2],
    thruster.generalizedLoadBody[6]};
  planar.currentVelocityNED = {0, 0, 0};
  shaft.command = 0.5;
  shaft.enabled = true;
  shaft.failed = false;
  rudderServo.command = 0;
  rudderServo.bias = 0;
  rudderServo.failed = false;
  mmg.generalizedForceBody = {
    thruster.generalizedLoadBody[1],
    thruster.generalizedLoadBody[2],
    thruster.generalizedLoadBody[6]};
  mmg.currentVelocityNED = {0, 0, 0};
  mmg.propellerRate = shaft.shaftRate;
  mmg.rudderAngle = rudderServo.position;
  spatial.generalizedForceBody = thruster.generalizedLoadBody;
  spatial.currentVelocityNED = {0, 0, 0};
  planarPosition = planar.poseNED[1];
  planarVelocity = planar.velocityBody[1];
  mmgPosition = mmg.poseNED[1];
  mmgVelocity = mmg.velocityBody[1];
  spatialPosition = spatial.positionNED[1];
  spatialVelocity = spatial.velocityBody[1];
end LoadIntegrationValidation;
