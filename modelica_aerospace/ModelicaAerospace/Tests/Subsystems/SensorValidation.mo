within ModelicaAerospace.Tests.Subsystems;
model SensorValidation "Exercise ideal and deterministic non-ideal sensors"
  ModelicaAerospace.Sensors.IdealVector idealVector;
  ModelicaAerospace.Sensors.VectorSensor vectorSensor(
    bias={0.1, -0.2, 0.3},
    noiseAmplitude={0.02, 0.03, 0.04},
    quantizationInterval={0.05, 0.05, 0.05},
    seed=13);
  ModelicaAerospace.Sensors.FirstOrderDelay delay(
    timeConstant=0.5,
    initialOutput=0);
  ModelicaAerospace.Sensors.IdealAirData idealAirData;
  ModelicaAerospace.Sensors.IdealInertial idealInertial;
  ModelicaAerospace.Sensors.GPS gps(
    positionBias={1, 2, 3},
    velocityBias={0.1, 0.2, 0.3});
  ModelicaAerospace.Sensors.Altimeter altimeter(
    bias=3,
    quantizationInterval=2);
  ModelicaAerospace.Sensors.Attitude attitude(
    bias={0, 0.1, 0, 0});
  output Real idealMeasurement[3];
  output Real vectorMeasurement[3];
  output Real delayedSignal;
  output Real idealAirDataMeasurement[4];
  output Real idealAcceleration[3];
  output Real idealRates[3];
  output Real gpsPosition[3];
  output Real gpsVelocity[3];
  output Real measuredAltitude;
  output Real measuredQuaternion[4];
equation
  idealVector.truth = {1, 2, 3};
  vectorSensor.truth = {1, 2, 3};
  idealMeasurement = idealVector.measurement;
  vectorMeasurement = vectorSensor.measurement;
  delay.u = if time < 1 then 0 else 1;
  delayedSignal = delay.y;

  idealAirData.trueAirspeed = 120;
  idealAirData.angleOfAttack = 0.1;
  idealAirData.sideslip = -0.02;
  idealAirData.altitude = 1500;
  idealAirDataMeasurement = {
    idealAirData.measuredAirspeed,
    idealAirData.measuredAngleOfAttack,
    idealAirData.measuredSideslip,
    idealAirData.measuredAltitude};

  idealInertial.accelerationBody = {1, -2, 3};
  idealInertial.angularVelocityBody = {0.1, 0.2, -0.3};
  idealAcceleration = idealInertial.measuredAccelerationBody;
  idealRates = idealInertial.measuredAngularVelocityBody;

  gps.positionECEF = {6378137, 10, 20};
  gps.velocityECEF = {100, -20, 5};
  gpsPosition = gps.measuredPositionECEF;
  gpsVelocity = gps.measuredVelocityECEF;
  altimeter.altitude = 100;
  measuredAltitude = altimeter.measuredAltitude;
  attitude.quaternion = {1, 0, 0, 0};
  measuredQuaternion = attitude.measuredQuaternion;
end SensorValidation;
