within ModelicaMaritime.Tests.VesselDynamics;
model FossenFreeDecay "Damped hydrostatic roll free-decay response"
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF vehicle(
    massProperties(
      mass=10,
      centerOfGravityBody={0, 0, 0.2},
      centerOfBuoyancyBody={0, 0, -0.2},
      inertiaBody=[5, 0, 0; 0, 6, 0; 0, 0, 7],
      displacedVolume=0.01),
    hydrodynamics(
      addedMass=diagonal({1, 1, 1, 1, 1, 1}),
      linearDamping=diagonal({1, 1, 1, 3, 3, 3}),
      quadraticDamping=zeros(6, 6)),
    initialState(
      positionNED={0, 0, 0},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={
        0.9950041652780258,
        0.09983341664682815,
        0,
        0},
      angularVelocityBody={0, 0, 0}),
    waterDensity=1000);
  output Real quaternion[4];
  output Real velocityBody[6];
  output Real dissipationPower;
equation
  vehicle.generalizedForceBody = zeros(6);
  vehicle.currentVelocityNED = {0, 0, 0};
  quaternion = vehicle.quaternionBodyToNED;
  velocityBody = vehicle.velocityBody;
  dissipationPower = vehicle.dissipationPower;
end FossenFreeDecay;
