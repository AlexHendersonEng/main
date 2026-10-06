within ModelicaAutomotive.Tests.PowertrainAerodynamics;
model TopSpeedEquilibrium "Drive force balances road load at target speed"
  parameter Real speed=30;
  parameter Real mass=1500;
  parameter Real rollingCoefficient=0.012;
  parameter Real dragArea=0.7;
  parameter Real airDensity=1.225;
  parameter Real wheelRadius=0.3;
  parameter Real ratio=6;
  parameter Real efficiency=0.95;
  parameter Real requiredForce =
    0.5 * airDensity * dragArea * speed * speed
    + rollingCoefficient * mass * 9.80665
      * speed / sqrt(speed * speed + 0.05 * 0.05);
  ModelicaAutomotive.Powertrain.IdealTorqueSource source(
    maximumDriveTorque=requiredForce * wheelRadius / (ratio * efficiency));
  ModelicaAutomotive.Powertrain.FixedRatio transmission(
    ratio=ratio,
    efficiency=efficiency);
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=mass,
    initialSpeed=speed,
    dragArea=dragArea,
    airDensity=airDensity,
    rollingResistanceCoefficient=rollingCoefficient,
    resistanceRegularization=0.05);
  output Real acceleration;
  output Real driveForce;
  output Real netForce;
equation
  source.command = 1;
  source.angularVelocity = transmission.inputAngularVelocity;
  transmission.inputTorque = source.torque;
  transmission.outputAngularVelocity = body.speed / wheelRadius;
  body.tireForce = transmission.outputTorque / wheelRadius;
  body.roadGrade = 0;
  body.windSpeed = 0;
  acceleration = body.acceleration;
  driveForce = body.tireForce;
  netForce = body.netForce;
end TopSpeedEquilibrium;
