within ModelicaAutomotive.Road;
function roadHeight "Evaluate locally parameterized road height"
  input ModelicaAutomotive.Types.Length longitudinalPosition;
  input ModelicaAutomotive.Types.Length lateralPosition;
  input ModelicaAutomotive.Types.RoadParameters road;
  output ModelicaAutomotive.Types.Length height;
algorithm
  assert(abs(road.grade) < 1.5707963267948966,
    "road grade magnitude must be less than pi/2");
  assert(abs(road.bank) < 1.5707963267948966,
    "road bank magnitude must be less than pi/2");
  height := road.referenceElevation
    + longitudinalPosition * tan(road.grade)
    + lateralPosition * tan(road.bank)
    + road.crown * lateralPosition * lateralPosition;
end roadHeight;
