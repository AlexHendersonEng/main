within ModelicaMaritime.Tests.VesselDynamics;
model MMGComponentValidation "Validate MMG hull, propeller, rudder, and load summation"
  ModelicaMaritime.VesselDynamics.MMG.MMGLoads loads(
    density=1000,
    referenceLength=10,
    referenceDraft=2,
    hullCoefficients(
      surgeConstant=-0.02,
      surgeSway2=-0.1,
      swayLinear=-0.5,
      swayYaw=0.1,
      yawSway=0.08,
      yawRate=-0.15),
    propellerProperties(
      diameter=2,
      wakeFraction=0.2,
      thrustDeduction=0.1,
      thrustCoefficient={0.25, -0.1, -0.05}),
    rudderProperties(
      area=3,
      longitudinalPosition=-4,
      hullForcePosition=-1,
      liftGradient=6,
      axialInflowFactor=1.1,
      lateralInflowFactor=0.8,
      hullForceIncrease=0.2,
      steeringResistanceDeduction=0.1));
  output Real hullLoad[3];
  output Real propellerLoad[3];
  output Real rudderLoad[3];
  output Real totalLoad[3];
  output Real componentSumError[3];
  output Real advanceRatio;
  output Real thrustCoefficient;
  output Real rudderAngleOfAttack;
equation
  loads.relativeVelocityBody = {5, 1, 0.1};
  loads.propellerRate = 2;
  loads.rudderAngle = 0.15;
  hullLoad = loads.hullLoadBody;
  propellerLoad = loads.propellerLoadBody;
  rudderLoad = loads.rudderLoadBody;
  totalLoad = loads.generalizedLoadBody;
  componentSumError = totalLoad - hullLoad - propellerLoad - rudderLoad;
  advanceRatio = loads.advanceRatio;
  thrustCoefficient = loads.thrustCoefficient;
  rudderAngleOfAttack = loads.rudderAngleOfAttack;
end MMGComponentValidation;
