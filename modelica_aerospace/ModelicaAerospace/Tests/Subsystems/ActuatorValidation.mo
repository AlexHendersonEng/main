within ModelicaAerospace.Tests.Subsystems;
model ActuatorValidation "Exercise actuator deadband, rate, position, and failure behavior"
  ModelicaAerospace.Actuators.Servo servo(
    timeConstant=0.1,
    minimumPosition=-1,
    maximumPosition=1,
    maximumRate=0.5,
    deadband=0.05,
    failurePosition=-0.25,
    initialPosition=0);
  ModelicaAerospace.Actuators.Servo failedServo(
    timeConstant=0.1,
    minimumPosition=-1,
    maximumPosition=1,
    maximumRate=0.5,
    deadband=0.05,
    failurePosition=-0.25,
    initialPosition=1);
  output Real position;
  output Real positionRate;
  output Real failedPosition;
equation
  servo.command = if time < 1 then -0.1 else 2;
  servo.bias = 0.1;
  servo.failed = false;
  position = servo.position;
  positionRate = servo.positionRate;
  failedServo.command = 1;
  failedServo.bias = 0;
  failedServo.failed = true;
  failedPosition = failedServo.position;
end ActuatorValidation;
