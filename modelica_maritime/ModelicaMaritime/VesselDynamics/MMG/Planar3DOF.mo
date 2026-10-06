within ModelicaMaritime.VesselDynamics.MMG;
block Planar3DOF "MMG-style planar vehicle composed over the generic 3-DoF plant"
  extends ModelicaMaritime.Interfaces.PartialPlanarVehicle;
  parameter ModelicaMaritime.Types.PlanarMassProperties massProperties;
  parameter ModelicaMaritime.Types.PlanarHydrodynamicProperties hydrodynamics;
  parameter ModelicaMaritime.Types.PlanarInitialState initialState;
  parameter ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter ModelicaMaritime.Types.Length referenceLength = 1;
  parameter ModelicaMaritime.Types.Length referenceDraft = 1;
  parameter ModelicaMaritime.Types.MMGHullCoefficients hullCoefficients;
  parameter ModelicaMaritime.Types.MMGPropellerProperties propellerProperties;
  parameter ModelicaMaritime.Types.MMGRudderProperties rudderProperties;
  ModelicaMaritime.Interfaces.Vector3Input currentVelocityNED(each unit="m/s");
  ModelicaMaritime.Interfaces.RealInput propellerRate(unit="1/s");
  ModelicaMaritime.Interfaces.RealInput rudderAngle(unit="rad");
  ModelicaMaritime.Interfaces.Vector3Output relativeVelocityBody;
  ModelicaMaritime.Interfaces.Vector3Output hullLoadBody;
  ModelicaMaritime.Interfaces.Vector3Output propellerLoadBody;
  ModelicaMaritime.Interfaces.Vector3Output rudderLoadBody;
  ModelicaMaritime.Interfaces.Vector3Output mmgLoadBody;
  ModelicaMaritime.Interfaces.Vector3Output accelerationBody;
protected
  ModelicaMaritime.VesselDynamics.Generic.Planar3DOF vehicle(
    massProperties=massProperties,
    hydrodynamics=hydrodynamics,
    initialState=initialState);
  replaceable ModelicaMaritime.VesselDynamics.MMG.MMGLoads loads(
    density=density,
    referenceLength=referenceLength,
    referenceDraft=referenceDraft,
    hullCoefficients=hullCoefficients,
    propellerProperties=propellerProperties,
    rudderProperties=rudderProperties)
    annotation (choicesAllMatching=true);
equation
  vehicle.currentVelocityNED = currentVelocityNED;
  loads.relativeVelocityBody = vehicle.relativeVelocityBody;
  loads.propellerRate = propellerRate;
  loads.rudderAngle = rudderAngle;
  vehicle.generalizedForceBody = generalizedForceBody + loads.generalizedLoadBody;
  poseNED = vehicle.poseNED;
  velocityBody = vehicle.velocityBody;
  relativeVelocityBody = vehicle.relativeVelocityBody;
  hullLoadBody = loads.hullLoadBody;
  propellerLoadBody = loads.propellerLoadBody;
  rudderLoadBody = loads.rudderLoadBody;
  mmgLoadBody = loads.generalizedLoadBody;
  accelerationBody = vehicle.accelerationBody;
end Planar3DOF;
