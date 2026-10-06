within ModelicaAutomotive.Examples;
model DoubleLaneChange "Closed-loop double-lane-change path following"
  ModelicaAutomotive.Scenarios.DoubleLaneChangePath path;
  ModelicaAutomotive.Drivers.LookAheadSteering driver(
    lookAheadTime=0.5,
    minimumLookAhead=4,
    headingGain=1.5,
    maximumSteeringAngle=0.45);
  ModelicaAutomotive.VehicleDynamics.Planar.KinematicBicycle vehicle;
  ModelicaAutomotive.Scenarios.ScenarioMetrics metrics;
  ModelicaAutomotive.Scenarios.TerminationCriteria termination(
    targetDistance=90,
    maximumDuration=8,
    maximumAbsoluteLateralError=1);
  output Real position[2];
  output Real yaw(unit="rad");
  output Real targetLateralPosition(unit="m");
  output Real lateralError(unit="m");
  output Real steeringCommand(unit="rad");
  output Real distanceTravelled(unit="m");
  output Real completion;
  output Real failure;
equation
  path.longitudinalPosition = vehicle.positionX;
  driver.lateralError = path.lateralPosition - vehicle.positionY;
  driver.headingError = path.heading - vehicle.yaw;
  driver.speed = 12;
  vehicle.speed = 12;
  vehicle.steeringAngle = driver.steeringCommand;
  metrics.speed = 12;
  metrics.lateralError = driver.lateralError;
  metrics.yawRate = vehicle.yawRate;
  metrics.controlEffort = driver.steeringCommand;
  termination.distanceTravelled = metrics.distanceTravelled;
  termination.lateralError = driver.lateralError;
  position = {vehicle.positionX, vehicle.positionY};
  yaw = vehicle.yaw;
  targetLateralPosition = path.lateralPosition;
  lateralError = driver.lateralError;
  steeringCommand = driver.steeringCommand;
  distanceTravelled = metrics.distanceTravelled;
  completion = termination.complete;
  failure = termination.failed;
  annotation (
    experiment(StartTime=0, StopTime=8, Tolerance=1e-8, Interval=0.01),
    Documentation(info="<html><p>A look-ahead driver follows a smooth out-and-back lane-change path while exposing pose, command, tracking, and completion outputs.</p></html>"));
end DoubleLaneChange;
