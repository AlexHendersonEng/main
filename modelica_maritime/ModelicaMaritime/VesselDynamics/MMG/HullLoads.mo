within ModelicaMaritime.VesselDynamics.MMG;
block HullLoads "Polynomial MMG-style hull loads"
  parameter ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter ModelicaMaritime.Types.Length referenceLength = 1;
  parameter ModelicaMaritime.Types.Length referenceDraft = 1;
  parameter ModelicaMaritime.Types.MMGHullCoefficients coefficients;
  ModelicaMaritime.Interfaces.Vector3Input relativeVelocityBody
    "Water-relative {u, v, r}";
  ModelicaMaritime.Interfaces.Vector3Output generalizedLoadBody "{XH, YH, NH}";
  ModelicaMaritime.Interfaces.RealOutput relativeSpeed(unit="m/s");
  ModelicaMaritime.Interfaces.RealOutput normalizedSway;
  ModelicaMaritime.Interfaces.RealOutput normalizedYawRate;
protected
  Real loadScale(unit="N");
  Real surgeCoefficient;
  Real swayCoefficient;
  Real yawCoefficient;
equation
  assert(density > 0, "MMG hull density must be positive");
  assert(referenceLength > 0, "MMG hull reference length must be positive");
  assert(referenceDraft > 0, "MMG hull reference draft must be positive");
  relativeSpeed = sqrt(
    relativeVelocityBody[1] * relativeVelocityBody[1]
    + relativeVelocityBody[2] * relativeVelocityBody[2]);
  normalizedSway = if relativeSpeed > ModelicaMaritime.Constants.small then
    relativeVelocityBody[2] / relativeSpeed else 0;
  normalizedYawRate = if relativeSpeed > ModelicaMaritime.Constants.small then
    relativeVelocityBody[3] * referenceLength / relativeSpeed else 0;
  surgeCoefficient =
    coefficients.surgeConstant
    + coefficients.surgeSway2 * normalizedSway * normalizedSway
    + coefficients.surgeSwayYaw * normalizedSway * normalizedYawRate
    + coefficients.surgeYaw2 * normalizedYawRate * normalizedYawRate
    + coefficients.surgeSway4 * normalizedSway * normalizedSway
      * normalizedSway * normalizedSway;
  swayCoefficient =
    coefficients.swayLinear * normalizedSway
    + coefficients.swayYaw * normalizedYawRate
    + coefficients.swayCubic * normalizedSway * normalizedSway * normalizedSway
    + coefficients.sway2Yaw * normalizedSway * normalizedSway * normalizedYawRate
    + coefficients.swayYaw2 * normalizedSway * normalizedYawRate * normalizedYawRate
    + coefficients.swayYawRateCubic * normalizedYawRate * normalizedYawRate
      * normalizedYawRate;
  yawCoefficient =
    coefficients.yawSway * normalizedSway
    + coefficients.yawRate * normalizedYawRate
    + coefficients.yawSwayCubic * normalizedSway * normalizedSway * normalizedSway
    + coefficients.yawSway2Rate * normalizedSway * normalizedSway * normalizedYawRate
    + coefficients.yawSwayRate2 * normalizedSway * normalizedYawRate
      * normalizedYawRate
    + coefficients.yawRateCubic * normalizedYawRate * normalizedYawRate
      * normalizedYawRate;
  loadScale = 0.5 * density * referenceLength * referenceDraft
    * relativeSpeed * relativeSpeed;
  generalizedLoadBody = {
    loadScale * surgeCoefficient,
    loadScale * swayCoefficient,
    loadScale * referenceLength * yawCoefficient};
end HullLoads;
