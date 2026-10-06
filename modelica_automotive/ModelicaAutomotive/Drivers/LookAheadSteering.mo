within ModelicaAutomotive.Drivers;
block LookAheadSteering "Look-ahead path steering from lateral and heading errors"
  parameter ModelicaAutomotive.Types.Length wheelbase = 2.7;
  parameter Real lookAheadTime(unit="s") = 0.8;
  parameter ModelicaAutomotive.Types.Length minimumLookAhead = 2;
  parameter Real headingGain = 1;
  parameter ModelicaAutomotive.Types.Angle maximumSteeringAngle = 0.6;
  ModelicaAutomotive.Interfaces.RealInput lateralError(unit="m")
    "Target path position minus vehicle position, positive left";
  ModelicaAutomotive.Interfaces.RealInput headingError(unit="rad")
    "Target path heading minus vehicle yaw";
  ModelicaAutomotive.Interfaces.RealInput speed(unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput steeringCommand(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput lookAheadDistance(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput curvatureCommand(unit="1/m");
protected
  Real unconstrainedSteering(unit="rad");
equation
  assert(wheelbase > 0, "wheelbase must be positive");
  assert(lookAheadTime > 0, "lookAheadTime must be positive");
  assert(minimumLookAhead > 0, "minimumLookAhead must be positive");
  assert(headingGain >= 0, "headingGain must not be negative");
  assert(maximumSteeringAngle > 0, "maximumSteeringAngle must be positive");
  lookAheadDistance = max(minimumLookAhead, abs(speed) * lookAheadTime);
  curvatureCommand =
    headingGain * headingError / lookAheadDistance
    + 2 * lateralError / (lookAheadDistance * lookAheadDistance);
  unconstrainedSteering = atan(wheelbase * curvatureCommand);
  steeringCommand = min(max(
    unconstrainedSteering,
    -maximumSteeringAngle),
    maximumSteeringAngle);
end LookAheadSteering;
