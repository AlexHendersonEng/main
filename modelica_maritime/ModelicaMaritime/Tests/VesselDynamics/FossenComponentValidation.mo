within ModelicaMaritime.Tests.VesselDynamics;
model FossenComponentValidation "Validate 6-DoF mass, Coriolis, damping, and hydrostatics"
  parameter ModelicaMaritime.Types.MassProperties properties(
    mass=10,
    centerOfGravityBody={0.1, -0.2, 0.3},
    centerOfBuoyancyBody={0, 0, -0.1},
    inertiaBody=[2, 0.1, 0; 0.1, 3, 0.2; 0, 0.2, 4],
    displacedVolume=0.01);
  parameter Real quaternion[4] = {
    0.9950041652780258,
    0.09983341664682815,
    0,
    0};
  output Real rotation[3, 3];
  output Real rigidBodyMass[6, 6];
  output Real coriolis[6, 6];
  output Real coriolisPower;
  output Real damping[6];
  output Real restoring[6];
  output Real quaternionRate[4];
equation
  rotation =
    ModelicaMaritime.Coordinates.quaternionBodyToNEDMatrix(quaternion);
  rigidBodyMass =
    ModelicaMaritime.VesselDynamics.Fossen.rigidBodyMassMatrix(properties);
  coriolis =
    ModelicaMaritime.VesselDynamics.Fossen.coriolisMatrix(
      rigidBodyMass,
      {2, -1, 0.5, 0.1, -0.2, 0.3});
  coriolisPower = {2, -1, 0.5, 0.1, -0.2, 0.3}
    * (coriolis * {2, -1, 0.5, 0.1, -0.2, 0.3});
  damping =
    ModelicaMaritime.VesselDynamics.Fossen.dampingLoad(
      diagonal({10, 20, 30, 40, 50, 60}),
      diagonal({1, 2, 3, 4, 5, 6}),
      {2, -1, 0.5, 0.1, -0.2, 0.3});
  restoring =
    ModelicaMaritime.VesselDynamics.Fossen.hydrostaticRestoringLoad(
      properties,
      quaternion,
      1000,
      9.80665);
  quaternionRate =
    ModelicaMaritime.Coordinates.quaternionDerivativeBodyToNED(
      quaternion,
      {0.1, -0.2, 0.3});
end FossenComponentValidation;
