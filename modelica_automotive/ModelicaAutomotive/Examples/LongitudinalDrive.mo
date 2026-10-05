within ModelicaAutomotive.Examples;
model LongitudinalDrive "Accelerate, release the drive force, and coast down"
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=1500,
    initialSpeed=10,
    dragArea=0.72,
    airDensity=1.225,
    rollingResistanceCoefficient=0.012,
    resistanceRegularization=0.05);
  output Real position;
  output Real speed;
  output Real acceleration;
  output Real tireForce;
equation
  tireForce = if time < 5 then 3500 else 0;
  body.tireForce = tireForce;
  body.roadGrade = 0;
  body.windSpeed = 0;
  position = body.position;
  speed = body.speed;
  acceleration = body.acceleration;
  annotation (
    experiment(StartTime=0, StopTime=20, Tolerance=1e-8, Interval=0.05),
    Documentation(info="<html>
<p>A passenger vehicle accelerates under a constant tire force for five
seconds and then coasts against aerodynamic drag and rolling resistance.</p>
</html>"));
end LongitudinalDrive;
