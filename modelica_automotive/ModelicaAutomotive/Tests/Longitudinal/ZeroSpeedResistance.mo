within ModelicaAutomotive.Tests.Longitudinal;
model ZeroSpeedResistance "Rolling resistance is finite and zero at rest"
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=1000,
    rollingResistanceCoefficient=0.02,
    resistanceRegularization=0.1);
  output Real rollingResistanceForce;
  output Real acceleration;
equation
  body.tireForce = 0;
  body.roadGrade = 0;
  body.windSpeed = 0;
  rollingResistanceForce = body.rollingResistanceForce;
  acceleration = body.acceleration;
end ZeroSpeedResistance;
