within ModelicaAutomotive.Tests.Longitudinal;
model GradeEquilibrium "Tire force balances grade, rolling, and drag resistance"
  parameter Real grade=0.08;
  parameter Real speed=20;
  parameter Real mass=1500;
  parameter Real gravity=9.80665;
  parameter Real dragArea=0.7;
  parameter Real airDensity=1.225;
  parameter Real rollingCoefficient=0.012;
  parameter Real regularization=0.01;
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=mass,
    initialSpeed=speed,
    dragArea=dragArea,
    airDensity=airDensity,
    rollingResistanceCoefficient=rollingCoefficient,
    resistanceRegularization=regularization);
  output Real acceleration;
  output Real netForce;
equation
  body.tireForce = 0.5 * airDensity * dragArea * speed * speed
    + rollingCoefficient * mass * gravity * cos(grade)
      * speed / sqrt(speed * speed + regularization * regularization)
    + mass * gravity * sin(grade);
  body.roadGrade = grade;
  body.windSpeed = 0;
  acceleration = body.acceleration;
  netForce = body.netForce;
end GradeEquilibrium;
