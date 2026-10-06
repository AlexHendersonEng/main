within ModelicaMaritime.Tests.Common;
model CommonValidation "Validate common types, records, constants, and interfaces"
  ModelicaMaritime.Types.MassProperties properties(
    mass=1200,
    displacedVolume=1200 / ModelicaMaritime.Constants.standardSeawaterDensity);
  ModelicaMaritime.Types.PlanarInitialState planarState;
  ModelicaMaritime.Types.RigidBodyInitialState rigidState;
  ModelicaMaritime.Types.WaterState water;
  ModelicaMaritime.Tests.Common.EnvironmentStub environment;
  ModelicaMaritime.Tests.Common.SensorStub sensor;
  ModelicaMaritime.Tests.Common.PlanarVehicleStub planarVehicle;
  ModelicaMaritime.Tests.Common.RigidBodyVehicleStub rigidVehicle;
  output Real environmentDensity;
  output Real sensorMeasurement;
  output Real planarVelocity[3];
  output Real rigidVelocity[6];
  output Real standardGravity;
  output Real defaultQuaternionScalar;
equation
  environment.depth = 10;
  environmentDensity = environment.density;
  sensor.truth = 4;
  sensorMeasurement = sensor.measurement;
  planarVehicle.generalizedForceBody = {7, 8, 9};
  planarVelocity = planarVehicle.velocityBody;
  rigidVehicle.generalizedForceBody = {1, 2, 3, 4, 5, 6};
  rigidVelocity = rigidVehicle.velocityBody;
  standardGravity = ModelicaMaritime.Constants.standardGravity;
  defaultQuaternionScalar = rigidState.quaternionBodyToNED[1];
  assert(properties.mass > 0, "Mass properties must be positive");
  assert(water.density > 0, "Water density must be positive");
  assert(planarState.heading == 0, "Default planar heading must be zero");
end CommonValidation;
