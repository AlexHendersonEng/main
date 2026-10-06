within ModelicaAutomotive.Steering;
block SteeringRatio "Convert steering-wheel angle to limited road-wheel angle"
  parameter Real ratio(min=ModelicaAutomotive.Constants.small) = 15;
  parameter ModelicaAutomotive.Types.Angle maximumRoadWheelAngle = 0.6;
  ModelicaAutomotive.Interfaces.RealInput steeringWheelAngle(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput roadWheelAngle(unit="rad");
equation
  assert(ratio > 0, "Steering ratio must be positive");
  assert(maximumRoadWheelAngle > 0, "maximumRoadWheelAngle must be positive");
  roadWheelAngle = min(
    max(steeringWheelAngle / ratio, -maximumRoadWheelAngle),
    maximumRoadWheelAngle);
end SteeringRatio;
