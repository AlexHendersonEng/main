within ModelicaAutomotive.Scenarios;
record PathDefinition "Double-lane-change path geometry"
  Real laneWidth(unit="m") = 3.5;
  Real entryStart(unit="m") = 15;
  Real transitionLength(unit="m") = 20;
  Real holdLength(unit="m") = 10;
end PathDefinition;
