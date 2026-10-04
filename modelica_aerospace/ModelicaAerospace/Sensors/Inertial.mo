within ModelicaAerospace.Sensors;
block Inertial "Configurable accelerometer and rate gyro"
  parameter Real accelerationBias[3] = {0, 0, 0};
  parameter Real rateBias[3] = {0, 0, 0};
  parameter Real accelerationNoise[3] = {0, 0, 0};
  parameter Real rateNoise[3] = {0, 0, 0};
  parameter Real accelerationQuantization[3] = {0, 0, 0};
  parameter Real rateQuantization[3] = {0, 0, 0};
  parameter Integer seed = 1;
  ModelicaAerospace.Interfaces.Vector3Input accelerationBody(each unit="m/s2");
  ModelicaAerospace.Interfaces.Vector3Input angularVelocityBody(each unit="rad/s");
  ModelicaAerospace.Interfaces.Vector3Output measuredAccelerationBody(each unit="m/s2");
  ModelicaAerospace.Interfaces.Vector3Output measuredAngularVelocityBody(each unit="rad/s");
protected
  ModelicaAerospace.Sensors.VectorSensor accelerometer(
    bias=accelerationBias,
    noiseAmplitude=accelerationNoise,
    quantizationInterval=accelerationQuantization,
    seed=seed);
  ModelicaAerospace.Sensors.VectorSensor gyro(
    bias=rateBias,
    noiseAmplitude=rateNoise,
    quantizationInterval=rateQuantization,
    seed=seed + 10);
equation
  accelerometer.truth = accelerationBody;
  gyro.truth = angularVelocityBody;
  measuredAccelerationBody = accelerometer.measurement;
  measuredAngularVelocityBody = gyro.measurement;
end Inertial;
