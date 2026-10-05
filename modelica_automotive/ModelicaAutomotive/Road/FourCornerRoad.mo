within ModelicaAutomotive.Road;
block FourCornerRoad "Query one road surface at four wheel contact points"
  parameter ModelicaAutomotive.Types.RoadParameters road;
  ModelicaAutomotive.Interfaces.CornerInput longitudinalPosition(each unit="m");
  ModelicaAutomotive.Interfaces.CornerInput lateralPosition(each unit="m");
  ModelicaAutomotive.Interfaces.CornerOutput height(each unit="m");
  ModelicaAutomotive.Interfaces.Vector3Output normalWorld;
  ModelicaAutomotive.Interfaces.RealOutput frictionCoefficient;
equation
  for corner in 1:4 loop
    height[corner] = road.referenceElevation
      + longitudinalPosition[corner] * tan(road.grade)
      + lateralPosition[corner] * tan(road.bank)
      + road.crown * lateralPosition[corner] * lateralPosition[corner];
  end for;
  normalWorld = ModelicaAutomotive.Road.roadNormal(road);
  frictionCoefficient = road.frictionCoefficient;
end FourCornerRoad;
