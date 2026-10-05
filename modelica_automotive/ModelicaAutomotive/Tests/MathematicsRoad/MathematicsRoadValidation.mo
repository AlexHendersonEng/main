within ModelicaAutomotive.Tests.MathematicsRoad;
model MathematicsRoadValidation "Validate mathematics helpers and road queries"
  parameter ModelicaAutomotive.Types.RoadParameters road(
    referenceElevation=10,
    heading=0.3,
    grade=0.1,
    bank=0.05,
    crown=0.02,
    frictionCoefficient=0.8);
  ModelicaAutomotive.Road.FourCornerRoad surface(road=road);
  Real dcm[3, 3];
  Real transformed[3];
  Real forces[4, 3];
  Real points[4, 3];
  Real totalForce[3];
  Real totalMoment[3];
  output Real wrapped;
  output Real zeroSign;
  output Real positiveSign;
  output Real roadHeights[4];
  output Real roadNormal[3];
  output Real transformedVector[3];
  output Real aggregateForce[3];
  output Real aggregateMoment[3];
equation
  surface.longitudinalPosition = {2, 2, -1, -1};
  surface.lateralPosition = {0.8, -0.8, 0.8, -0.8};
  roadHeights = surface.height;
  roadNormal = surface.normalWorld;
  wrapped = ModelicaAutomotive.Mathematics.wrapAngle(4);
  zeroSign = ModelicaAutomotive.Mathematics.regularizedSign(0, 0.1);
  positiveSign = ModelicaAutomotive.Mathematics.regularizedSign(1, 0.1);
  dcm = ModelicaAutomotive.Mathematics.roadToWorldMatrix(0.3, 0.1, 0.05);
  transformed = ModelicaAutomotive.Mathematics.transformVector(dcm, {1, 0, 0});
  transformedVector = transformed;
  forces = [10, 0, 0; 10, 0, 0; 10, 0, 0; 10, 0, 0];
  points = [1, 1, 0; 1, -1, 0; -1, 1, 0; -1, -1, 0];
  (totalForce, totalMoment) =
    ModelicaAutomotive.Mathematics.aggregateForcesMoments(forces, points);
  aggregateForce = totalForce;
  aggregateMoment = totalMoment;
end MathematicsRoadValidation;
