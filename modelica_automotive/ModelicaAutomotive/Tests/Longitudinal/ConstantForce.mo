within ModelicaAutomotive.Tests.Longitudinal;
model ConstantForce "Constant-force analytic motion"
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=1000,
    initialSpeed=5,
    initialPosition=2);
  output Real position;
  output Real speed;
  output Real acceleration;
equation
  body.tireForce = 2000;
  body.roadGrade = 0;
  body.windSpeed = 0;
  position = body.position;
  speed = body.speed;
  acceleration = body.acceleration;
end ConstantForce;
