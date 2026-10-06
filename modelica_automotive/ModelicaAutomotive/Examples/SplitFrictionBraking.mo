within ModelicaAutomotive.Examples;
model SplitFrictionBraking "Two-wheel braking on unequal-friction surfaces"
  parameter Real vehicleMass(unit="kg")=1500;
  parameter Real track(unit="m")=1.6;
  parameter Real rollingRadius(unit="m")=0.3;
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
  ModelicaAutomotive.Brakes.IdealBrake leftBrake(maximumTorque=1800);
  ModelicaAutomotive.Brakes.IdealBrake rightBrake(maximumTorque=1800);
  ModelicaAutomotive.Tires.Kinematics leftKinematics(rollingRadius=rollingRadius);
  ModelicaAutomotive.Tires.Kinematics rightKinematics(rollingRadius=rollingRadius);
  ModelicaAutomotive.Tires.LinearTire leftTire;
  ModelicaAutomotive.Tires.LinearTire rightTire;
  output Real speed(unit="m/s");
  output Real leftLongitudinalForce(unit="N");
  output Real rightLongitudinalForce(unit="N");
  output Real yawMoment(unit="N.m");
equation
  leftBrake.command = 0.8;
  rightBrake.command = 0.8;
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
  speed = body.speed;
  leftLongitudinalForce = leftTire.longitudinalForce;
  rightLongitudinalForce = rightTire.longitudinalForce;
  yawMoment = track / 2
    * (rightTire.longitudinalForce - leftTire.longitudinalForce);
  annotation (
    experiment(StartTime=0, StopTime=3, Tolerance=1e-8, Interval=0.005),
    Documentation(info="<html>
<p>Applies equal brake commands to left and right wheels on high- and
low-friction surfaces, exposing the resulting longitudinal-force imbalance
and yaw moment.</p>
</html>"));
end SplitFrictionBraking;
