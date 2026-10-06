within ModelicaMaritime.Tests.VesselDynamics;
model PlanarConstantForce "Analytic surge motion with rigid and added inertia"
  ModelicaMaritime.VesselDynamics.Generic.Planar3DOF vessel(
    massProperties(
      mass=10,
      centerOfGravityX=0,
      yawInertia=20),
    hydrodynamics(
      addedMass=[5, 0, 0; 0, 0, 0; 0, 0, 0],
      linearDamping=zeros(3, 3),
      quadraticDamping=zeros(3, 3)),
    initialState(
      positionNED={0, 0},
      heading=0,
      velocityBody={2, 0},
      yawRate=0));
  output Real poseNED[3];
  output Real velocityBody[3];
  output Real accelerationBody[3];
equation
  vessel.generalizedForceBody = {30, 0, 0};
  vessel.currentVelocityNED = {0, 0, 0};
  poseNED = vessel.poseNED;
  velocityBody = vessel.velocityBody;
  accelerationBody = vessel.accelerationBody;
end PlanarConstantForce;
