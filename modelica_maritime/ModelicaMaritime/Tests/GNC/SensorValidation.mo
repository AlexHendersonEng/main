within ModelicaMaritime.Tests.GNC;
model SensorValidation "Validate ideal and deterministic maritime sensors"
  ModelicaMaritime.Sensors.IdealScalar idealScalar;
  ModelicaMaritime.Sensors.IdealVector idealVector;
  ModelicaMaritime.Sensors.ScalarSensor scalar(
    bias=0.2,
    noiseAmplitude=0.1,
    quantizationInterval=0.05,
    seed=4);
  ModelicaMaritime.Sensors.VectorSensor vector(
    bias={0.1, -0.2, 0.3},
    noiseAmplitude={0.05, 0.05, 0.05},
    quantizationInterval={0.02, 0.02, 0.02},
    seed=5);
  ModelicaMaritime.Sensors.PositionVelocity positionVelocity(
    positionBias={1, 2, 3},
    velocityBias={0.1, 0.2, 0.3});
  ModelicaMaritime.Sensors.Heading heading(bias=0.1);
  ModelicaMaritime.Sensors.SpeedLog speedLog(surgeBias=0.2);
  ModelicaMaritime.Sensors.Inertial inertial(
    accelerationBias={0.1, 0.2, 0.3},
    rateBias={0.01, 0.02, 0.03});
  ModelicaMaritime.Sensors.PressureDepth depth(
    waterDensity=1025,
    pressureBias=100);
  ModelicaMaritime.Sensors.Altitude altitude(bias=0.5);
  ModelicaMaritime.Sensors.RangeBearing rangeBearing(
    rangeBias=1,
    bearingBias=0.1);
  output Real idealScalarMeasurement;
  output Real idealVectorMeasurement[3];
  output Real scalarMeasurement;
  output Real vectorMeasurement[3];
  output Real measuredPosition[3];
  output Real measuredVelocity[3];
  output Real measuredHeading;
  output Real measuredSpeed;
  output Real measuredSurge;
  output Real measuredAcceleration[3];
  output Real measuredRates[3];
  output Real measuredPressure;
  output Real measuredDepth;
  output Real measuredAltitude;
  output Real measuredRange;
  output Real measuredBearing;
equation
  idealScalar.truth = 4;
  idealVector.truth = {1, 2, 3};
  scalar.truth = 2;
  vector.truth = {1, 2, 3};
  positionVelocity.positionNED = {10, 20, 30};
  positionVelocity.velocityNED = {1, 2, 3};
  heading.heading = 6.25;
  speedLog.relativeVelocityBody = {3, 4, 0};
  inertial.accelerationBody = {1, 2, 3};
  inertial.angularVelocityBody = {0.1, 0.2, 0.3};
  depth.depth = 20;
  altitude.vehicleDepth = 30;
  altitude.seafloorDepth = 100;
  rangeBearing.observerPositionNED = {10, 20, 0};
  rangeBearing.targetPositionNED = {13, 24, 0};
  idealScalarMeasurement = idealScalar.measurement;
  idealVectorMeasurement = idealVector.measurement;
  scalarMeasurement = scalar.measurement;
  vectorMeasurement = vector.measurement;
  measuredPosition = positionVelocity.measuredPositionNED;
  measuredVelocity = positionVelocity.measuredVelocityNED;
  measuredHeading = heading.measuredHeading;
  measuredSpeed = speedLog.measuredSpeed;
  measuredSurge = speedLog.measuredSurge;
  measuredAcceleration = inertial.measuredAccelerationBody;
  measuredRates = inertial.measuredAngularVelocityBody;
  measuredPressure = depth.measuredPressure;
  measuredDepth = depth.measuredDepth;
  measuredAltitude = altitude.measuredAltitude;
  measuredRange = rangeBearing.measuredRange;
  measuredBearing = rangeBearing.measuredBearing;
end SensorValidation;
