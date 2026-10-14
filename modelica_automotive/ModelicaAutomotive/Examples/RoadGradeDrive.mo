within ModelicaAutomotive.Examples;
model RoadGradeDrive "Longitudinal drive over a crowned, banked uphill road"
  parameter ModelicaAutomotive.Types.RoadParameters roadParameters(
    referenceElevation=150,
    grade=0.06,
    bank=0.04,
    crown=0.001,
    frictionCoefficient=0.7);
  ModelicaAutomotive.Road.ConstantRoad road(road=roadParameters);
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body vehicle(
    mass=1600,
    initialSpeed=8,
    dragArea=0.72,
    rollingResistanceCoefficient=0.012,
    resistanceRegularization=0.05);
  output Real position(unit="m");
  output Real lateralPosition(unit="m");
  output Real roadHeight(unit="m");
  output Real normalWorld[3];
  output Real speed(unit="m/s");
  output Real acceleration(unit="m/s2");
  output Real gradeForce(unit="N");
  output Real frictionCoefficient;
equation
  position = vehicle.position;
  lateralPosition = 1.5 * sin(0.08 * position);
  road.longitudinalPosition = position;
  road.lateralPosition = lateralPosition;
  vehicle.tireForce = if time < 4 then 2800 else 0;
  vehicle.roadGrade = roadParameters.grade;
  vehicle.windSpeed = 0;
  roadHeight = road.height;
  normalWorld = road.normalWorld;
  speed = vehicle.speed;
  acceleration = vehicle.acceleration;
  gradeForce = vehicle.gradeForce;
  frictionCoefficient = road.frictionCoefficient;
  annotation (
    experiment(StartTime=0, StopTime=16, Tolerance=1e-8, Interval=0.02),
    Documentation(info="<html>
<p>Combines a longitudinal vehicle, a locally analytic road surface, and an
uphill traction pulse. Road height and surface normal vary with position over
a crowned, banked grade; the vehicle exposes speed and grade-load response.</p>
</html>"));
end RoadGradeDrive;
