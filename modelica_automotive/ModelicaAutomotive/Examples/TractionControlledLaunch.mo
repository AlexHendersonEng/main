within ModelicaAutomotive.Examples;
model TractionControlledLaunch "Driven-axle launch with traction control"
  parameter Real vehicleMass(unit="kg") = 1500;
  parameter Real rollingRadius(unit="m") = 0.32;
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=vehicleMass,
    initialSpeed=2,
    dragArea=0.7,
    rollingResistanceCoefficient=0.01);
  ModelicaAutomotive.Wheels.RotationalDynamics leftWheel(
    inertia=1.4,
    rollingRadius=rollingRadius,
    initialAngularVelocity=2 / rollingRadius);
  ModelicaAutomotive.Wheels.RotationalDynamics rightWheel(
    inertia=1.4,
    rollingRadius=rollingRadius,
    initialAngularVelocity=2 / rollingRadius);
  ModelicaAutomotive.Tires.Kinematics leftKinematics(rollingRadius=rollingRadius);
  ModelicaAutomotive.Tires.Kinematics rightKinematics(rollingRadius=rollingRadius);
  ModelicaAutomotive.Tires.LinearTire leftTire;
  ModelicaAutomotive.Tires.LinearTire rightTire;
  ModelicaAutomotive.Control.TractionControl controller(
    activationSlip=0.08,
    maximumSlip=0.2,
    minimumCommand=0.1);
  output Real speed(unit="m/s");
  output Real propulsionCommand;
  output Real slipRatio[2];
  output Real tireForce[2];
equation
  controller.propulsionCommand = 1;
  controller.slipRatio = {
    leftKinematics.slipRatio,
    rightKinematics.slipRatio,
    0,
    0};
  leftKinematics.longitudinalVelocity = body.speed;
  leftKinematics.lateralVelocity = 0;
  leftKinematics.angularVelocity = leftWheel.angularVelocity;
  rightKinematics.longitudinalVelocity = body.speed;
  rightKinematics.lateralVelocity = 0;
  rightKinematics.angularVelocity = rightWheel.angularVelocity;
  leftTire.slipRatio = leftKinematics.slipRatio;
  leftTire.slipAngle = 0;
  leftTire.normalLoad = vehicleMass * 9.80665 / 2;
  leftTire.frictionCoefficient = 0.45;
  rightTire.slipRatio = rightKinematics.slipRatio;
  rightTire.slipAngle = 0;
  rightTire.normalLoad = vehicleMass * 9.80665 / 2;
  rightTire.frictionCoefficient = 0.45;
  leftWheel.hubTorque = 1600 * controller.limitedPropulsionCommand;
  leftWheel.brakeTorque = 0;
  leftWheel.longitudinalForce = leftTire.longitudinalForce;
  rightWheel.hubTorque = 1600 * controller.limitedPropulsionCommand;
  rightWheel.brakeTorque = 0;
  rightWheel.longitudinalForce = rightTire.longitudinalForce;
  body.tireForce = leftTire.longitudinalForce + rightTire.longitudinalForce;
  body.roadGrade = 0;
  body.windSpeed = 0;
  speed = body.speed;
  propulsionCommand = controller.limitedPropulsionCommand;
  slipRatio = {leftKinematics.slipRatio, rightKinematics.slipRatio};
  tireForce = {leftTire.longitudinalForce, rightTire.longitudinalForce};
  annotation (
    experiment(StartTime=0, StopTime=6, Tolerance=1e-8, Interval=0.005),
    Documentation(info="<html><p>A low-friction driven axle launches under slip-based traction-control torque reduction.</p></html>"));
end TractionControlledLaunch;
