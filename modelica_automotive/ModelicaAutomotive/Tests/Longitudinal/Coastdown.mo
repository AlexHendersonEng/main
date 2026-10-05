within ModelicaAutomotive.Tests.Longitudinal;
model Coastdown "Quadratic-drag analytic coastdown"
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=1200,
    initialSpeed=30,
    dragArea=0.8,
    airDensity=1.2);
  output Real position;
  output Real speed;
  output Real acceleration;
  output Real aerodynamicForce;
equation
  body.tireForce = 0;
  body.roadGrade = 0;
  body.windSpeed = 0;
  position = body.position;
  speed = body.speed;
  acceleration = body.acceleration;
  aerodynamicForce = body.aerodynamicForce;
end Coastdown;
