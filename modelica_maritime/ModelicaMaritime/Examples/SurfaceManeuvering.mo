within ModelicaMaritime.Examples;
model SurfaceManeuvering "Generic surface-vessel acceleration and turning in current"
  ModelicaMaritime.VesselDynamics.Generic.Planar3DOF vessel(
    massProperties(
      mass=5000,
      centerOfGravityX=0.5,
      yawInertia=200000),
    hydrodynamics(
      addedMass=[1000, 0, 0; 0, 3000, 0; 0, 0, 50000],
      linearDamping=[1200, 0, 0; 0, 5000, 0; 0, 0, 80000],
      quadraticDamping=[400, 0, 0; 0, 1500, 0; 0, 0, 20000]),
    initialState(
      positionNED={0, 0},
      heading=0,
      velocityBody={0, 0},
      yawRate=0));
  output Real poseNED[3];
  output Real velocityBody[3];
  output Real relativeVelocityBody[3];
  output Real dissipationPower;
equation
  vessel.generalizedForceBody = {
    if time < 10 then 8000 else 5000,
    0,
    if time >= 15 and time < 35 then 25000 else 0};
  vessel.currentVelocityNED = {0.5, 0.2, 0};
  poseNED = vessel.poseNED;
  velocityBody = vessel.velocityBody;
  relativeVelocityBody = vessel.relativeVelocityBody;
  dissipationPower = vessel.dissipationPower;
  annotation (
    experiment(StartTime=0, StopTime=60, Tolerance=1e-8, Interval=0.1),
    Documentation(info="<html>
<p>Accelerates a generic surface vessel, applies a bounded yaw moment, and
continues in a steady NED current. The example exercises added inertia,
Coriolis terms, current-relative damping, and planar kinematics.</p>
</html>"));
end SurfaceManeuvering;
