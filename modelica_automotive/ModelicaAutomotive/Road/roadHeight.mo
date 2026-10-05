within ModelicaAutomotive.Road;
function roadHeight "Evaluate locally parameterized road height"
  input ModelicaAutomotive.Types.Length longitudinalPosition;
  input ModelicaAutomotive.Types.Length lateralPosition;
  input ModelicaAutomotive.Types.RoadParameters road;
  output ModelicaAutomotive.Types.Length height;
algorithm
  height := road.referenceElevation
    + longitudinalPosition * tan(road.grade)
    + lateralPosition * tan(road.bank)
    + road.crown * lateralPosition * lateralPosition;
end roadHeight;
