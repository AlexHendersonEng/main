within ModelicaMaritime.Sensors;
block PositionVelocity "Configurable NED position and ground-velocity sensor"
  parameter Real positionBias[3] = {0, 0, 0};
  parameter Real velocityBias[3] = {0, 0, 0};
  parameter Real positionNoise[3] = {0, 0, 0};
  parameter Real velocityNoise[3] = {0, 0, 0};
  parameter Real positionQuantization[3] = {0, 0, 0};
  parameter Real velocityQuantization[3] = {0, 0, 0};
  parameter Integer seed = 1;
  ModelicaMaritime.Interfaces.Vector3Input positionNED(each unit="m");
  ModelicaMaritime.Interfaces.Vector3Input velocityNED(each unit="m/s");
  ModelicaMaritime.Interfaces.Vector3Output measuredPositionNED(each unit="m");
  ModelicaMaritime.Interfaces.Vector3Output measuredVelocityNED(each unit="m/s");
protected
  ModelicaMaritime.Sensors.VectorSensor positionSensor(
    bias=positionBias,
    noiseAmplitude=positionNoise,
    quantizationInterval=positionQuantization,
    seed=seed);
  ModelicaMaritime.Sensors.VectorSensor velocitySensor(
    bias=velocityBias,
    noiseAmplitude=velocityNoise,
    quantizationInterval=velocityQuantization,
    seed=seed + 10);
equation
  positionSensor.truth = positionNED;
  velocitySensor.truth = velocityNED;
  measuredPositionNED = positionSensor.measurement;
  measuredVelocityNED = velocitySensor.measurement;
end PositionVelocity;
