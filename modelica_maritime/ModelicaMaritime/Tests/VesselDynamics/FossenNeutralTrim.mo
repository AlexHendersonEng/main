within ModelicaMaritime.Tests.VesselDynamics;
model FossenNeutralTrim "Neutral-buoyancy rest equilibrium"
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF vehicle(
    massProperties(
      mass=10,
      centerOfGravityBody={0, 0, 0},
      centerOfBuoyancyBody={0, 0, 0},
      inertiaBody=[2, 0, 0; 0, 3, 0; 0, 0, 4],
      displacedVolume=0.01),
    hydrodynamics(
      addedMass=diagonal({1, 1, 1, 1, 1, 1}),
      linearDamping=diagonal({2, 2, 2, 2, 2, 2}),
      quadraticDamping=zeros(6, 6)),
    initialState(
      positionNED={1, 2, 3},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    waterDensity=1000);
  output Real positionNED[3];
  output Real velocityBody[6];
  output Real restoringLoadBody[6];
  output Real quaternionNorm;
equation
  vehicle.generalizedForceBody = zeros(6);
  vehicle.currentVelocityNED = {0, 0, 0};
  positionNED = vehicle.positionNED;
  velocityBody = vehicle.velocityBody;
  restoringLoadBody = vehicle.restoringLoadBody;
  quaternionNorm = vehicle.quaternionNorm;
end FossenNeutralTrim;
