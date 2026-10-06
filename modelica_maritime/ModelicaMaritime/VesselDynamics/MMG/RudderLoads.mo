within ModelicaMaritime.VesselDynamics.MMG;
block RudderLoads "MMG-style rudder normal force and hull interaction"
  parameter ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter ModelicaMaritime.Types.MMGRudderProperties properties;
  ModelicaMaritime.Interfaces.Vector3Input relativeVelocityBody
    "Water-relative {u, v, r}";
  ModelicaMaritime.Interfaces.RealInput rudderAngle(unit="rad")
    "Positive angle produces positive yaw moment for an aft rudder";
  ModelicaMaritime.Interfaces.RealInput propellerAxialVelocity(unit="m/s");
  ModelicaMaritime.Interfaces.Vector3Output generalizedLoadBody "{XR, YR, NR}";
  ModelicaMaritime.Interfaces.RealOutput inflowVelocity[2]
    "Rudder axial and lateral inflow {uR, vR}";
  ModelicaMaritime.Interfaces.RealOutput angleOfAttack(unit="rad");
  ModelicaMaritime.Interfaces.RealOutput normalForce(unit="N");
protected
  Real inflowSpeed(unit="m/s");
equation
  assert(density > 0, "MMG rudder density must be positive");
  assert(properties.area > 0, "MMG rudder area must be positive");
  assert(properties.liftGradient >= 0, "MMG rudder lift gradient must be non-negative");
  assert(
    properties.steeringResistanceDeduction >= 0
      and properties.steeringResistanceDeduction < 1,
    "MMG rudder steering-resistance deduction must be in [0, 1)");
  inflowVelocity = {
    properties.axialInflowFactor * propellerAxialVelocity,
    properties.lateralInflowFactor * (
      relativeVelocityBody[2]
      + properties.longitudinalPosition * relativeVelocityBody[3])};
  inflowSpeed = sqrt(
    inflowVelocity[1] * inflowVelocity[1]
    + inflowVelocity[2] * inflowVelocity[2]);
  angleOfAttack = rudderAngle - atan2(inflowVelocity[2], inflowVelocity[1]);
  normalForce = 0.5 * density * properties.area * inflowSpeed * inflowSpeed
    * properties.liftGradient * sin(angleOfAttack);
  generalizedLoadBody = {
    -(1 - properties.steeringResistanceDeduction)
      * normalForce * sin(rudderAngle),
    -(1 + properties.hullForceIncrease)
      * normalForce * cos(rudderAngle),
    -(properties.longitudinalPosition
      + properties.hullForceIncrease * properties.hullForcePosition)
      * normalForce * cos(rudderAngle)};
end RudderLoads;
