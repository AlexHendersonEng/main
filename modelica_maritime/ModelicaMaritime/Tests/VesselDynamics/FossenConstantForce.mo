within ModelicaMaritime.Tests.VesselDynamics;
model FossenConstantForce "Analytic surge motion with rigid and added inertia"
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF vehicle(
    massProperties(
      mass=10,
      inertiaBody=[2, 0, 0; 0, 3, 0; 0, 0, 4],
      displacedVolume=0),
    hydrodynamics(
      addedMass=diagonal({5, 5, 5, 1, 1, 1}),
      linearDamping=zeros(6, 6),
      quadraticDamping=zeros(6, 6)),
    initialState(
      positionNED={0, 0, 0},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    gravity=0);
  output Real positionNED[3];
  output Real velocityBody[6];
  output Real accelerationBody[6];
equation
  vehicle.generalizedForceBody = {30, 0, 0, 0, 0, 0};
  vehicle.currentVelocityNED = {0, 0, 0};
  positionNED = vehicle.positionNED;
  velocityBody = vehicle.velocityBody;
  accelerationBody = vehicle.accelerationBody;
end FossenConstantForce;
