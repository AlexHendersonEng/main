within ModelicaAutomotive.Road;
function roadNormal "Unit road-surface normal without local crown curvature"
  input ModelicaAutomotive.Types.RoadParameters road;
  output ModelicaAutomotive.Types.Vector3 normalWorld;
protected
  ModelicaAutomotive.Types.Matrix3 roadToWorld;
algorithm
  roadToWorld := ModelicaAutomotive.Mathematics.roadToWorldMatrix(
    road.heading,
    road.grade,
    road.bank);
  normalWorld := roadToWorld[:, 3];
end roadNormal;
