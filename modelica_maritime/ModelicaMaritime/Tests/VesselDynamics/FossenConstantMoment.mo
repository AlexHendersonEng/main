within ModelicaMaritime.Tests.VesselDynamics;
model FossenConstantMoment "Analytic principal-axis roll motion"
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF vehicle(
    massProperties(
      mass=10,
      inertiaBody=[2, 0, 0; 0, 3, 0; 0, 0, 4],
      displacedVolume=0),
    hydrodynamics(
      addedMass=diagonal({1, 1, 1, 2, 1, 1}),
      linearDamping=zeros(6, 6),
      quadraticDamping=zeros(6, 6)),
    initialState(
      positionNED={0, 0, 0},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    gravity=0);
  output Real velocityBody[6];
  output Real accelerationBody[6];
  output Real quaternion[4];
  output Real quaternionNorm;
equation
  vehicle.generalizedForceBody = {0, 0, 0, 4, 0, 0};
  vehicle.currentVelocityNED = {0, 0, 0};
  velocityBody = vehicle.velocityBody;
  accelerationBody = vehicle.accelerationBody;
  quaternion = vehicle.quaternionBodyToNED;
  quaternionNorm = vehicle.quaternionNorm;
end FossenConstantMoment;
