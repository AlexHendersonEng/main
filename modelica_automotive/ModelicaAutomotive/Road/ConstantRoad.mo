within ModelicaAutomotive.Road;
block ConstantRoad "Locally constant grade, bank, crown, and friction surface"
  parameter ModelicaAutomotive.Types.RoadParameters road;
  ModelicaAutomotive.Interfaces.RealInput longitudinalPosition(unit="m");
  ModelicaAutomotive.Interfaces.RealInput lateralPosition(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput height(unit="m");
  ModelicaAutomotive.Interfaces.Vector3Output normalWorld;
  ModelicaAutomotive.Interfaces.RealOutput frictionCoefficient;
equation
  height = ModelicaAutomotive.Road.roadHeight(
    longitudinalPosition,
    lateralPosition,
    road);
  normalWorld = ModelicaAutomotive.Road.roadNormal(road);
  frictionCoefficient = road.frictionCoefficient;
end ConstantRoad;
