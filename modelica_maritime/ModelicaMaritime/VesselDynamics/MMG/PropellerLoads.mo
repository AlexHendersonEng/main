within ModelicaMaritime.VesselDynamics.MMG;
block PropellerLoads "Open-water MMG-style propeller thrust"
  parameter ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter ModelicaMaritime.Types.MMGPropellerProperties properties;
  ModelicaMaritime.Interfaces.Vector3Input relativeVelocityBody
    "Water-relative {u, v, r}";
  ModelicaMaritime.Interfaces.RealInput rotationRate(unit="1/s")
    "Signed propeller revolutions per second";
  ModelicaMaritime.Interfaces.Vector3Output generalizedLoadBody "{XP, 0, 0}";
  ModelicaMaritime.Interfaces.RealOutput advanceRatio;
  ModelicaMaritime.Interfaces.RealOutput thrustCoefficient;
  ModelicaMaritime.Interfaces.RealOutput axialVelocity(unit="m/s")
    "Nominal axial velocity at the propeller";
  ModelicaMaritime.Interfaces.RealOutput thrust(unit="N");
equation
  assert(density > 0, "MMG propeller density must be positive");
  assert(properties.diameter > 0, "MMG propeller diameter must be positive");
  assert(
    properties.wakeFraction >= 0 and properties.wakeFraction < 1,
    "MMG propeller wake fraction must be in [0, 1)");
  assert(
    properties.thrustDeduction >= 0 and properties.thrustDeduction < 1,
    "MMG propeller thrust deduction must be in [0, 1)");
  axialVelocity = (1 - properties.wakeFraction) * relativeVelocityBody[1];
  advanceRatio = if abs(rotationRate) > ModelicaMaritime.Constants.small then
    axialVelocity / (abs(rotationRate) * properties.diameter) else 0;
  thrustCoefficient =
    properties.thrustCoefficient[1]
    + properties.thrustCoefficient[2] * advanceRatio
    + properties.thrustCoefficient[3] * advanceRatio * advanceRatio;
  thrust = density * rotationRate * abs(rotationRate)
    * properties.diameter * properties.diameter
    * properties.diameter * properties.diameter * thrustCoefficient;
  generalizedLoadBody = {
    (1 - properties.thrustDeduction) * thrust,
    0,
    0};
end PropellerLoads;
