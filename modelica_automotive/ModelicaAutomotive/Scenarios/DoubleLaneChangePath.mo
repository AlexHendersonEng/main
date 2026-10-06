within ModelicaAutomotive.Scenarios;
block DoubleLaneChangePath "Smooth reference path for an out-and-back lane change"
  parameter ModelicaAutomotive.Scenarios.PathDefinition path;
  ModelicaAutomotive.Interfaces.RealInput longitudinalPosition(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput lateralPosition(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput heading(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput slope;
protected
  Real returnStart(unit="m");
equation
  assert(path.laneWidth > 0, "laneWidth must be positive");
  assert(path.entryStart >= 0, "entryStart must not be negative");
  assert(path.transitionLength > 0, "transitionLength must be positive");
  assert(path.holdLength >= 0, "holdLength must not be negative");
  returnStart = path.entryStart + path.transitionLength + path.holdLength;
  lateralPosition =
    if longitudinalPosition < path.entryStart then 0
    elseif longitudinalPosition < path.entryStart + path.transitionLength then
      0.5 * path.laneWidth * (1 - cos(
        3.141592653589793 * (longitudinalPosition - path.entryStart)
        / path.transitionLength))
    elseif longitudinalPosition < returnStart then path.laneWidth
    elseif longitudinalPosition < returnStart + path.transitionLength then
      0.5 * path.laneWidth * (1 + cos(
        3.141592653589793 * (longitudinalPosition - returnStart)
        / path.transitionLength))
    else 0;
  slope =
    if longitudinalPosition < path.entryStart then 0
    elseif longitudinalPosition < path.entryStart + path.transitionLength then
      0.5 * path.laneWidth * 3.141592653589793 / path.transitionLength
      * sin(3.141592653589793
        * (longitudinalPosition - path.entryStart) / path.transitionLength)
    elseif longitudinalPosition < returnStart then 0
    elseif longitudinalPosition < returnStart + path.transitionLength then
      -0.5 * path.laneWidth * 3.141592653589793 / path.transitionLength
      * sin(3.141592653589793
        * (longitudinalPosition - returnStart) / path.transitionLength)
    else 0;
  heading = atan(slope);
end DoubleLaneChangePath;
