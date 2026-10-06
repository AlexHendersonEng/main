within ModelicaMaritime.Sensors;
block Inertial "Configurable accelerometer and rate gyro"
  parameter Real accelerationBias[3] = {0, 0, 0};
  parameter Real rateBias[3] = {0, 0, 0};
  parameter Real accelerationNoise[3] = {0, 0, 0};
  parameter Real rateNoise[3] = {0, 0, 0};
  parameter Integer seed = 1;
  ModelicaMaritime.Interfaces.Vector3Input accelerationBody(each unit="m/s2");
  ModelicaMaritime.Interfaces.Vector3Input angularVelocityBody(each unit="rad/s");
  ModelicaMaritime.Interfaces.Vector3Output measuredAccelerationBody(each unit="m/s2");
  ModelicaMaritime.Interfaces.Vector3Output measuredAngularVelocityBody(each unit="rad/s");
protected
  ModelicaMaritime.Sensors.VectorSensor accelerometer(
    bias=accelerationBias,
    noiseAmplitude=accelerationNoise,
    seed=seed);
  ModelicaMaritime.Sensors.VectorSensor gyro(
    bias=rateBias,
    noiseAmplitude=rateNoise,
    seed=seed + 10);
equation
  accelerometer.truth = accelerationBody;
  gyro.truth = angularVelocityBody;
  measuredAccelerationBody = accelerometer.measurement;
  measuredAngularVelocityBody = gyro.measurement;
end Inertial;
