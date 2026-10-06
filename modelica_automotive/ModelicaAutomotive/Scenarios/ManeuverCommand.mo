within ModelicaAutomotive.Scenarios;
block ManeuverCommand "Bounded propulsion, brake, and steering pulse sequence"
  parameter Real startTime(unit="s") = 1;
  parameter Real holdTime(unit="s") = 2;
  parameter Real recoveryTime(unit="s") = 2;
  parameter Real propulsionLevel(min=0, max=1) = 0;
  parameter Real brakeLevel(min=0, max=1) = 0;
  parameter Real steeringLevel(unit="rad") = 0.05;
  ModelicaAutomotive.Interfaces.RealOutput propulsionCommand;
  ModelicaAutomotive.Interfaces.RealOutput brakeCommand;
  ModelicaAutomotive.Interfaces.RealOutput steeringCommand(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput phase
    "Zero before, one during hold, two during recovery, three complete";
equation
  assert(startTime >= 0, "startTime must not be negative");
  assert(holdTime >= 0, "holdTime must not be negative");
  assert(recoveryTime >= 0, "recoveryTime must not be negative");
  propulsionCommand =
    if time >= startTime and time < startTime + holdTime then propulsionLevel
    else 0;
  brakeCommand =
    if time >= startTime and time < startTime + holdTime then brakeLevel
    else 0;
  steeringCommand =
    if time >= startTime and time < startTime + holdTime then steeringLevel
    elseif time < startTime + holdTime + recoveryTime
      and time >= startTime + holdTime then -steeringLevel
    else 0;
  phase =
    if time < startTime then 0
    elseif time < startTime + holdTime then 1
    elseif time < startTime + holdTime + recoveryTime then 2
    else 3;
end ManeuverCommand;
