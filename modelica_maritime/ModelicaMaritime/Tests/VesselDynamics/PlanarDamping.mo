within ModelicaMaritime.Tests.VesselDynamics;
model PlanarDamping "Linear surge damping and kinetic-energy decay"
  ModelicaMaritime.VesselDynamics.Generic.Planar3DOF vessel(
    massProperties(
      mass=10,
      centerOfGravityX=0,
      yawInertia=20),
    hydrodynamics(
      addedMass=zeros(3, 3),
      linearDamping=[2, 0, 0; 0, 3, 0; 0, 0, 4],
      quadraticDamping=zeros(3, 3)),
    initialState(
      positionNED={0, 0},
      heading=0,
      velocityBody={4, 0},
      yawRate=0));
  output Real poseNED[3];
  output Real velocityBody[3];
  output Real kineticEnergy;
  output Real dissipationPower;
equation
  vessel.generalizedForceBody = {0, 0, 0};
  vessel.currentVelocityNED = {0, 0, 0};
  poseNED = vessel.poseNED;
  velocityBody = vessel.velocityBody;
  kineticEnergy = vessel.kineticEnergy;
  dissipationPower = vessel.dissipationPower;
end PlanarDamping;
