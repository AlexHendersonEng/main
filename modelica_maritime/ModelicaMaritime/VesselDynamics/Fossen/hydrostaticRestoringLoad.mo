within ModelicaMaritime.VesselDynamics.Fossen;
function hydrostaticRestoringLoad "Weight and buoyancy load expressed in body axes"
  input ModelicaMaritime.Types.MassProperties massProperties;
  input ModelicaMaritime.Types.Quaternion quaternionBodyToNED;
  input ModelicaMaritime.Types.Density waterDensity =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  input Real gravity(unit="m/s2") =
    ModelicaMaritime.Constants.standardGravity;
  output ModelicaMaritime.Types.GeneralizedForce loadBody;
protected
  Real normalized[4];
  Real quaternionNorm;
  Real rotationBodyToNED[3, 3];
  Real weightBody[3];
  Real buoyancyBody[3];
  Real momentBody[3];
  Real weight(unit="N");
  Real buoyancy(unit="N");
algorithm
  assert(waterDensity > 0, "Water density must be positive");
  assert(gravity >= 0, "Gravity must be non-negative");
  assert(massProperties.displacedVolume >= 0, "Displaced volume must be non-negative");
  quaternionNorm := sqrt(
    quaternionBodyToNED[1] * quaternionBodyToNED[1]
    + quaternionBodyToNED[2] * quaternionBodyToNED[2]
    + quaternionBodyToNED[3] * quaternionBodyToNED[3]
    + quaternionBodyToNED[4] * quaternionBodyToNED[4]);
  assert(quaternionNorm > 0, "Quaternion norm must be positive");
  normalized := quaternionBodyToNED / quaternionNorm;
  rotationBodyToNED :=
    ModelicaMaritime.Coordinates.quaternionBodyToNEDMatrix(normalized);
  weight := massProperties.mass * gravity;
  buoyancy := waterDensity * massProperties.displacedVolume * gravity;
  weightBody := transpose(rotationBodyToNED) * {0, 0, weight};
  buoyancyBody := transpose(rotationBodyToNED) * {0, 0, -buoyancy};
  momentBody :=
    cross(massProperties.centerOfGravityBody, weightBody)
    + cross(massProperties.centerOfBuoyancyBody, buoyancyBody);
  loadBody := {
    weightBody[1] + buoyancyBody[1],
    weightBody[2] + buoyancyBody[2],
    weightBody[3] + buoyancyBody[3],
    momentBody[1],
    momentBody[2],
    momentBody[3]};
end hydrostaticRestoringLoad;
