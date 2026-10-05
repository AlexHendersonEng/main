within ModelicaAutomotive.VehicleDynamics.Longitudinal;
block Body "One-dimensional longitudinal vehicle dynamics"
  parameter ModelicaAutomotive.Types.Mass mass = 1500;
  parameter ModelicaAutomotive.Types.Velocity initialSpeed = 0;
  parameter ModelicaAutomotive.Types.Length initialPosition = 0;
  parameter Real dragArea(unit="m2", min=0) = 0
    "Drag coefficient multiplied by frontal area";
  parameter Real airDensity(unit="kg/m3", min=0) = 1.225;
  parameter Real rollingResistanceCoefficient(min=0) = 0;
  parameter ModelicaAutomotive.Types.Acceleration gravity =
    ModelicaAutomotive.Constants.standardGravity;
  parameter ModelicaAutomotive.Types.Velocity resistanceRegularization = 0.01;
  ModelicaAutomotive.Interfaces.RealInput tireForce(unit="N")
    "Net longitudinal tire force, positive forward";
  ModelicaAutomotive.Interfaces.RealInput roadGrade(unit="rad")
    "Positive for an uphill road";
  ModelicaAutomotive.Interfaces.RealInput windSpeed(unit="m/s")
    "World-relative wind velocity, positive in the travel direction";
  ModelicaAutomotive.Interfaces.RealOutput position(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput speed(unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput acceleration(unit="m/s2");
  ModelicaAutomotive.Interfaces.RealOutput aerodynamicForce(unit="N")
    "Aerodynamic force opposing relative air velocity";
  ModelicaAutomotive.Interfaces.RealOutput rollingResistanceForce(unit="N")
    "Rolling resistance opposing vehicle velocity";
  ModelicaAutomotive.Interfaces.RealOutput gradeForce(unit="N")
    "Gravity component opposing positive uphill travel";
  ModelicaAutomotive.Interfaces.RealOutput netForce(unit="N");
protected
  Real positionState(start=initialPosition, fixed=true);
  Real speedState(start=initialSpeed, fixed=true);
  Real relativeAirSpeed(unit="m/s");
equation
  assert(mass > 0, "Longitudinal body mass must be positive");
  assert(dragArea >= 0, "dragArea must not be negative");
  assert(airDensity >= 0, "airDensity must not be negative");
  assert(rollingResistanceCoefficient >= 0,
    "rollingResistanceCoefficient must not be negative");
  assert(gravity > 0, "gravity must be positive");
  assert(resistanceRegularization > 0,
    "resistanceRegularization must be positive");
  relativeAirSpeed = speedState - windSpeed;
  aerodynamicForce = 0.5 * airDensity * dragArea
    * relativeAirSpeed * abs(relativeAirSpeed);
  rollingResistanceForce = rollingResistanceCoefficient * mass * gravity
    * cos(roadGrade) * ModelicaAutomotive.Mathematics.regularizedSign(
      speedState,
      resistanceRegularization);
  gradeForce = mass * gravity * sin(roadGrade);
  netForce = tireForce - aerodynamicForce - rollingResistanceForce - gradeForce;
  acceleration = netForce / mass;
  der(positionState) = speedState;
  der(speedState) = acceleration;
  position = positionState;
  speed = speedState;
end Body;
