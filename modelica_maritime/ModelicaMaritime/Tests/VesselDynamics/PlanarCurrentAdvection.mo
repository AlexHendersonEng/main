within ModelicaMaritime.Tests.VesselDynamics;
model PlanarCurrentAdvection "Zero-relative-speed vessel advected by steady current"
  ModelicaMaritime.VesselDynamics.Generic.Planar3DOF vessel(
    massProperties(
      mass=50,
      centerOfGravityX=0,
      yawInertia=100),
    hydrodynamics(
      addedMass=zeros(3, 3),
      linearDamping=[20, 0, 0; 0, 30, 0; 0, 0, 40],
      quadraticDamping=zeros(3, 3)),
    initialState(
      positionNED={0, 0},
      heading=0,
      velocityBody={2, 0},
      yawRate=0));
  output Real poseNED[3];
  output Real velocityBody[3];
  output Real relativeVelocityBody[3];
  output Real dissipationPower;
equation
  vessel.generalizedForceBody = {0, 0, 0};
  vessel.currentVelocityNED = {2, 0, 0};
  poseNED = vessel.poseNED;
  velocityBody = vessel.velocityBody;
  relativeVelocityBody = vessel.relativeVelocityBody;
  dissipationPower = vessel.dissipationPower;
end PlanarCurrentAdvection;
