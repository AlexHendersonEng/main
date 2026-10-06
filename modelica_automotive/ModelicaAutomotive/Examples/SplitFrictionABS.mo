within ModelicaAutomotive.Examples;
model SplitFrictionABS "Split-friction braking with independent ABS modulation"
  parameter Real vehicleMass(unit="kg") = 1500;
  parameter Real rollingRadius(unit="m") = 0.3;
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=vehicleMass,
    initialSpeed=20);
  ModelicaAutomotive.Wheels.RotationalDynamics leftWheel(
    inertia=1.5,
    rollingRadius=rollingRadius,
    initialAngularVelocity=20 / rollingRadius);
  ModelicaAutomotive.Wheels.RotationalDynamics rightWheel(
    inertia=1.5,
    rollingRadius=rollingRadius,
    initialAngularVelocity=20 / rollingRadius);
  ModelicaAutomotive.Brakes.IdealBrake leftBrake(maximumTorque=2200);
  ModelicaAutomotive.Brakes.IdealBrake rightBrake(maximumTorque=2200);
  ModelicaAutomotive.Tires.Kinematics leftKinematics(rollingRadius=rollingRadius);
  ModelicaAutomotive.Tires.Kinematics rightKinematics(rollingRadius=rollingRadius);
  ModelicaAutomotive.Tires.LinearTire leftTire;
  ModelicaAutomotive.Tires.LinearTire rightTire;
  ModelicaAutomotive.Control.AntiLockBraking controller(
    activationSlip=0.08,
    lockedSlip=0.24,
    minimumCommand=0.03);
  ModelicaAutomotive.Scenarios.ScenarioMetrics metrics;
  output Real speed(unit="m/s");
  output Real slipRatio[2];
  output Real brakeCommand[2];
  output Real longitudinalForce[2];
  output Real yawMoment(unit="N.m");
  output Real distanceTravelled(unit="m");
equation
  controller.brakeCommand = 1;
  controller.slipRatio = {
    leftKinematics.slipRatio,
    rightKinematics.slipRatio,
    leftKinematics.slipRatio,
    rightKinematics.slipRatio};
  leftBrake.command = controller.wheelBrakeCommand[1];
  rightBrake.command = controller.wheelBrakeCommand[2];
  leftBrake.angularVelocity = leftWheel.angularVelocity;
  rightBrake.angularVelocity = rightWheel.angularVelocity;
  leftKinematics.longitudinalVelocity = body.speed;
  leftKinematics.lateralVelocity = 0;
  leftKinematics.angularVelocity = leftWheel.angularVelocity;
  rightKinematics.longitudinalVelocity = body.speed;
  rightKinematics.lateralVelocity = 0;
  rightKinematics.angularVelocity = rightWheel.angularVelocity;
  leftTire.slipRatio = leftKinematics.slipRatio;
  leftTire.slipAngle = 0;
  leftTire.normalLoad = vehicleMass * 9.80665 / 2;
  leftTire.frictionCoefficient = 0.9;
  rightTire.slipRatio = rightKinematics.slipRatio;
  rightTire.slipAngle = 0;
  rightTire.normalLoad = vehicleMass * 9.80665 / 2;
  rightTire.frictionCoefficient = 0.3;
  leftWheel.hubTorque = 0;
  leftWheel.brakeTorque = leftBrake.brakeTorque;
  leftWheel.longitudinalForce = leftTire.longitudinalForce;
  rightWheel.hubTorque = 0;
  rightWheel.brakeTorque = rightBrake.brakeTorque;
  rightWheel.longitudinalForce = rightTire.longitudinalForce;
  body.tireForce = leftTire.longitudinalForce + rightTire.longitudinalForce;
  body.roadGrade = 0;
  body.windSpeed = 0;
  metrics.speed = body.speed;
  metrics.lateralError = 0;
  metrics.yawRate = 0;
  metrics.controlEffort =
    0.5 * (controller.wheelBrakeCommand[1]
      + controller.wheelBrakeCommand[2]);
  speed = body.speed;
  slipRatio = {leftKinematics.slipRatio, rightKinematics.slipRatio};
  brakeCommand = {
    controller.wheelBrakeCommand[1],
    controller.wheelBrakeCommand[2]};
  longitudinalForce = {
    leftTire.longitudinalForce,
    rightTire.longitudinalForce};
  yawMoment = 0.8
    * (rightTire.longitudinalForce - leftTire.longitudinalForce);
  distanceTravelled = metrics.distanceTravelled;
  annotation (
    experiment(StartTime=0, StopTime=4, Tolerance=1e-8, Interval=0.005),
    Documentation(info="<html><p>Independent wheel-speed dynamics and tire forces demonstrate ABS modulation on unequal left/right friction.</p></html>"));
end SplitFrictionABS;
