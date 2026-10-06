within ModelicaMaritime.Tests.VesselDynamics;
model HydrodynamicsValidation "Validate planar mass, Coriolis, damping, and coefficient loads"
  parameter ModelicaMaritime.Types.PlanarMassProperties properties(
    mass=100,
    centerOfGravityX=0.5,
    yawInertia=80);
  ModelicaMaritime.Hydrodynamics.CoefficientLoads coefficientBlock(
    density=1000,
    referenceLength=10,
    referenceDraft=2);
  output Real rigidBodyMass[3, 3];
  output Real coriolis[3, 3];
  output Real coriolisPower;
  output Real dampingLoad[3];
  output Real dampingPower;
  output Real functionCoefficientLoad[3];
  output Real blockCoefficientLoad[3];
equation
  rigidBodyMass =
    ModelicaMaritime.Hydrodynamics.planarRigidBodyMassMatrix(properties);
  coriolis = ModelicaMaritime.Hydrodynamics.planarCoriolisMatrix(
    rigidBodyMass,
    {3, -2, 0.4});
  coriolisPower = {3, -2, 0.4} * (coriolis * {3, -2, 0.4});
  dampingLoad = ModelicaMaritime.Hydrodynamics.planarDampingLoad(
    [10, 0, 0; 0, 20, 0; 0, 0, 30],
    [2, 0, 0; 0, 3, 0; 0, 0, 4],
    {3, -2, 0.5});
  dampingPower = -{3, -2, 0.5} * dampingLoad;
  functionCoefficientLoad =
    ModelicaMaritime.Hydrodynamics.coefficientPlanarLoads(
      {0.1, -0.2, 0.03},
      1000,
      10,
      2,
      5);
  coefficientBlock.coefficients = {0.1, -0.2, 0.03};
  coefficientBlock.relativeVelocityBody = {3, 4, 0.2};
  blockCoefficientLoad = coefficientBlock.generalizedLoad;
end HydrodynamicsValidation;
