within ModelicaAutomotive.Scenarios;
block TerminationCriteria "Report scenario completion and tracking-limit failure"
  parameter Real targetDistance(unit="m") = 100;
  parameter Real maximumDuration(unit="s") = 10;
  parameter Real maximumAbsoluteLateralError(unit="m") = 2;
  ModelicaAutomotive.Interfaces.RealInput distanceTravelled(unit="m");
  ModelicaAutomotive.Interfaces.RealInput lateralError(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput complete;
  ModelicaAutomotive.Interfaces.RealOutput failed;
  ModelicaAutomotive.Interfaces.RealOutput remainingDistance(unit="m");
equation
  assert(targetDistance >= 0, "targetDistance must not be negative");
  assert(maximumDuration > 0, "maximumDuration must be positive");
  assert(maximumAbsoluteLateralError > 0,
    "maximumAbsoluteLateralError must be positive");
  complete =
    if distanceTravelled >= targetDistance or time >= maximumDuration then 1
    else 0;
  failed =
    if abs(lateralError) > maximumAbsoluteLateralError then 1 else 0;
  remainingDistance = max(targetDistance - distanceTravelled, 0);
end TerminationCriteria;
