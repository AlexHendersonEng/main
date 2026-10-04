block DoubleSignal "Concrete scalar transform used to validate common connectors"
  extends ModelicaAerospace.Interfaces.PartialScalarTransform;
equation
  y = 2 * u;
end DoubleSignal;

block DoubleVector "Concrete vector transform used to validate array connectors"
  extends ModelicaAerospace.Interfaces.PartialVectorTransform;
equation
  y = 2 * u;
end DoubleVector;

block EnvironmentStub "Concrete environment interface"
  extends ModelicaAerospace.Interfaces.PartialEnvironment;
equation
  temperature = 288.15 - 0.0065 * altitude;
  pressure = 101325;
  density = 1.225;
  speedOfSound = 340.294;
end EnvironmentStub;

block SensorStub "Concrete sensor interface"
  extends ModelicaAerospace.Interfaces.PartialSensor;
equation
  measurement = truth;
end SensorStub;

block VehicleStub "Concrete vehicle-dynamics interface"
  extends ModelicaAerospace.Interfaces.PartialVehicleDynamics;
equation
  positionNED = {0, 0, 0};
  velocityBody = forceBody;
  quaternionBodyToNED = {1, 0, 0, 0};
  angularVelocityBody = momentBody;
end VehicleStub;

model CommonValidation "Validate common types, records, constants, and interfaces"
  ModelicaAerospace.Types.GeodeticPosition location(
    latitude=0.5,
    longitude=-1,
    altitude=1200);
  ModelicaAerospace.Types.MassProperties properties(mass=1200);
  ModelicaAerospace.Types.RotationalState rotation;
  DoubleSignal transform;
  DoubleVector vectorTransform;
  EnvironmentStub environment;
  SensorStub sensor;
  VehicleStub vehicle;
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
