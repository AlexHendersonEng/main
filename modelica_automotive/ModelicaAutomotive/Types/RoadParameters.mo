within ModelicaAutomotive.Types;
record RoadParameters "Locally constant road surface parameters"
  ModelicaAutomotive.Types.Length referenceElevation = 0
    "Road height at the world-frame origin";
  ModelicaAutomotive.Types.Angle heading = 0
    "Road x-axis heading measured counter-clockwise from world X";
  ModelicaAutomotive.Types.Angle grade = 0
    "Positive when road height increases along road x";
  ModelicaAutomotive.Types.Angle bank = 0
    "Positive when the left side of the road is higher";
  Real crown(unit="1/m") = 0
    "Quadratic road crown coefficient applied to lateral distance squared";
  Real frictionCoefficient(min=0) = 1
    "Nominal tire-road friction coefficient";
end RoadParameters;
