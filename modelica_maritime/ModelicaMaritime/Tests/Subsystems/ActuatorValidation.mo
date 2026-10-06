within ModelicaMaritime.Tests.Subsystems;
model ActuatorValidation "Validate servo, control surfaces, ballast, and buoyancy"
  ModelicaMaritime.Actuators.Servo servo(
    minimumPosition=-1,
    maximumPosition=1,
    risingRate=0.5,
    fallingRate=-0.5,
    deadband=0.05,
    failurePosition=-0.25);
  ModelicaMaritime.Actuators.ControlSurface rudder(
    density=1025,
    area=2,
    liftSlope=5,
    dragCoefficient=0.02,
    normalDirectionBody={0, 1, 0},
    applicationPointBody={-4, 0, 0});
  ModelicaMaritime.Actuators.ControlSurface hydroplane(
    density=1025,
    area=1.5,
    liftSlope=4,
    dragCoefficient=0.01,
    normalDirectionBody={0, 0, 1},
    applicationPointBody={2, 0, 0});
  ModelicaMaritime.Actuators.BallastTank ballast(
    capacity=2,
    initialMass=0,
    maximumFillRate=1,
    maximumEmptyRate=0.5);
  ModelicaMaritime.Actuators.VariableBuoyancy buoyancy(
    minimumVolume=0.4,
    maximumVolume=0.8,
    initialVolume=0.5,
    maximumVolumeRate=0.1,
    waterDensity=1025);
  output Real servoPosition;
  output Real servoRate;
  output Real rudderLoad[6];
  output Real hydroplaneLoad[6];
  output Real ballastMass;
  output Real ballastLoad[6];
  output Real displacedVolume;
  output Real buoyancyLoad[6];
equation
  servo.command = if time < 1 then 0.02 else 1;
  servo.bias = 0;
  servo.failed = time >= 4;
  rudder.deflection = 0.1;
  rudder.relativeVelocityBody = {4, 0, 0};
  hydroplane.deflection = -0.08;
  hydroplane.relativeVelocityBody = {3, 0, 0};
  ballast.pumpCommand = if time < 3 then 1 else -0.5;
  ballast.failed = false;
  buoyancy.command = 0.5;
  buoyancy.failed = false;
  servoPosition = servo.position;
  servoRate = servo.positionRate;
  rudderLoad = rudder.generalizedLoadBody;
  hydroplaneLoad = hydroplane.generalizedLoadBody;
  ballastMass = ballast.ballastMass;
  ballastLoad = ballast.generalizedLoadBody;
  displacedVolume = buoyancy.displacedVolume;
  buoyancyLoad = buoyancy.generalizedLoadBody;
end ActuatorValidation;
