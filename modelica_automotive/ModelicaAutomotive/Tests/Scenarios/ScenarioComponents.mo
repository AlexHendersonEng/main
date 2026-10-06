within ModelicaAutomotive.Tests.Scenarios;
model ScenarioComponents "Reusable path, command, metric, and termination validation"
  ModelicaAutomotive.Scenarios.DoubleLaneChangePath path(
    path(
      laneWidth=4,
      entryStart=10,
      transitionLength=20,
      holdLength=10));
  ModelicaAutomotive.Scenarios.ManeuverCommand command(
    startTime=1,
    holdTime=2,
    recoveryTime=1,
    propulsionLevel=0.6,
    brakeLevel=0.4,
    steeringLevel=0.1);
  ModelicaAutomotive.Scenarios.ScenarioMetrics metrics;
  ModelicaAutomotive.Scenarios.TerminationCriteria termination(
    targetDistance=5,
    maximumDuration=10,
    maximumAbsoluteLateralError=2);
  output Real lateralPosition;
  output Real heading;
  output Real propulsionCommand;
  output Real brakeCommand;
  output Real steeringCommand;
  output Real phase;
  output Real distanceTravelled;
  output Real absoluteLateralErrorIntegral;
  output Real complete;
  output Real failed;
  output Real headingIntegral(start=0, fixed=true);
equation
  path.longitudinalPosition = 20;
  metrics.speed = 2;
  metrics.lateralError = 0.5;
  metrics.yawRate = 0.2;
  metrics.controlEffort = 0.25;
  termination.distanceTravelled = metrics.distanceTravelled;
  termination.lateralError = metrics.lateralError;
  lateralPosition = path.lateralPosition;
  heading = path.heading;
  propulsionCommand = command.propulsionCommand;
  brakeCommand = command.brakeCommand;
  steeringCommand = command.steeringCommand;
  phase = command.phase;
  distanceTravelled = metrics.distanceTravelled;
  absoluteLateralErrorIntegral = metrics.absoluteLateralErrorIntegral;
  complete = termination.complete;
  failed = termination.failed;
  der(headingIntegral) = path.heading;
end ScenarioComponents;
