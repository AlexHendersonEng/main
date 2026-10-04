within ModelicaAerospace.Tests.Common;
model CommonValidation "Validate common types, records, constants, and interfaces"
  ModelicaAerospace.Types.GeodeticPosition location(
    latitude=0.5,
    longitude=-1,
    altitude=1200);
  ModelicaAerospace.Types.MassProperties properties(mass=1200);
  ModelicaAerospace.Types.RotationalState rotation;
  ModelicaAerospace.Tests.Common.DoubleSignal transform;
  ModelicaAerospace.Tests.Common.DoubleVector vectorTransform;
  ModelicaAerospace.Tests.Common.EnvironmentStub environment;
  ModelicaAerospace.Tests.Common.SensorStub sensor;
  ModelicaAerospace.Tests.Common.VehicleStub vehicle;
  output Real y;
  output Real vectorY[3];
  output Real environmentDensity;
  output Real sensorMeasurement[3];
  output Real vehicleVelocity[3];
  output Real semiMajorAxis;
  output Real defaultQuaternionScalar;
equation
  transform.u = 3;
  y = transform.y;
  vectorTransform.u = {1, 2, 3};
  vectorY = vectorTransform.y;
  environment.altitude = 1000;
  environmentDensity = environment.density;
  sensor.truth = {4, 5, 6};
  sensorMeasurement = sensor.measurement;
  vehicle.forceBody = {7, 8, 9};
  vehicle.momentBody = {0.1, 0.2, 0.3};
  vehicleVelocity = vehicle.velocityBody;
  semiMajorAxis = ModelicaAerospace.Constants.WGS84.semiMajorAxis;
  defaultQuaternionScalar = rotation.quaternion[1];
  assert(properties.mass > 0, "Default mass properties must be positive");
  assert(location.latitude >= -1.570796326794897, "Latitude lower bound");
end CommonValidation;
