within ModelicaMaritime.VesselDynamics.MMG;
block MMGLoads "Compose MMG hull, propeller, and rudder loads"
  parameter ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter ModelicaMaritime.Types.Length referenceLength = 1;
  parameter ModelicaMaritime.Types.Length referenceDraft = 1;
  parameter ModelicaMaritime.Types.MMGHullCoefficients hullCoefficients;
  parameter ModelicaMaritime.Types.MMGPropellerProperties propellerProperties;
  parameter ModelicaMaritime.Types.MMGRudderProperties rudderProperties;
  ModelicaMaritime.Interfaces.Vector3Input relativeVelocityBody
    "Water-relative {u, v, r}";
  ModelicaMaritime.Interfaces.RealInput propellerRate(unit="1/s");
  ModelicaMaritime.Interfaces.RealInput rudderAngle(unit="rad");
  ModelicaMaritime.Interfaces.Vector3Output hullLoadBody;
  ModelicaMaritime.Interfaces.Vector3Output propellerLoadBody;
  ModelicaMaritime.Interfaces.Vector3Output rudderLoadBody;
  ModelicaMaritime.Interfaces.Vector3Output generalizedLoadBody;
  ModelicaMaritime.Interfaces.RealOutput advanceRatio;
  ModelicaMaritime.Interfaces.RealOutput thrustCoefficient;
  ModelicaMaritime.Interfaces.RealOutput rudderAngleOfAttack(unit="rad");
protected
  replaceable ModelicaMaritime.VesselDynamics.MMG.HullLoads hull(
    density=density,
    referenceLength=referenceLength,
    referenceDraft=referenceDraft,
    coefficients=hullCoefficients)
    annotation (choicesAllMatching=true);
  replaceable ModelicaMaritime.VesselDynamics.MMG.PropellerLoads propeller(
    density=density,
    properties=propellerProperties)
    annotation (choicesAllMatching=true);
  replaceable ModelicaMaritime.VesselDynamics.MMG.RudderLoads rudder(
    density=density,
    properties=rudderProperties)
    annotation (choicesAllMatching=true);
equation
  hull.relativeVelocityBody = relativeVelocityBody;
  propeller.relativeVelocityBody = relativeVelocityBody;
  propeller.rotationRate = propellerRate;
  rudder.relativeVelocityBody = relativeVelocityBody;
  rudder.rudderAngle = rudderAngle;
  rudder.propellerAxialVelocity = propeller.axialVelocity;
  hullLoadBody = hull.generalizedLoadBody;
  propellerLoadBody = propeller.generalizedLoadBody;
  rudderLoadBody = rudder.generalizedLoadBody;
  generalizedLoadBody = hullLoadBody + propellerLoadBody + rudderLoadBody;
  advanceRatio = propeller.advanceRatio;
  thrustCoefficient = propeller.thrustCoefficient;
  rudderAngleOfAttack = rudder.angleOfAttack;
end MMGLoads;
