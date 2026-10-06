within ModelicaMaritime.Tests.VesselDynamics;
model MMGStraightEquilibrium "Straight-ahead equilibrium from balanced hull drag and thrust"
  ModelicaMaritime.VesselDynamics.MMG.Planar3DOF vessel(
    massProperties(
      mass=10000,
      centerOfGravityX=0,
      yawInertia=200000),
    hydrodynamics(
      addedMass=zeros(3, 3),
      linearDamping=zeros(3, 3),
      quadraticDamping=zeros(3, 3)),
    initialState(
      positionNED={0, 0},
      heading=0,
      velocityBody={5, 0},
      yawRate=0),
    density=1025,
    referenceLength=10,
    referenceDraft=2,
    hullCoefficients(
      surgeConstant=-0.02,
      swayLinear=-0.5,
      yawRate=-0.2),
    propellerProperties(
      diameter=1,
      wakeFraction=0,
      thrustDeduction=0,
      thrustCoefficient={0.2, 0, 0}),
    rudderProperties(
      area=2,
      longitudinalPosition=-4,
      liftGradient=6));
  output Real poseNED[3];
  output Real velocityBody[3];
  output Real accelerationBody[3];
  output Real mmgLoadBody[3];
equation
  vessel.generalizedForceBody = {0, 0, 0};
  vessel.currentVelocityNED = {0, 0, 0};
  vessel.propellerRate = 5;
  vessel.rudderAngle = 0;
  poseNED = vessel.poseNED;
  velocityBody = vessel.velocityBody;
  accelerationBody = vessel.accelerationBody;
  mmgLoadBody = vessel.mmgLoadBody;
end MMGStraightEquilibrium;
