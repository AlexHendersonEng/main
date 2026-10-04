within ModelicaAerospace.Sensors;
block GPS "Configurable deterministic ECEF position and velocity sensor"
  parameter Real positionBias[3] = {0, 0, 0};
  parameter Real velocityBias[3] = {0, 0, 0};
  parameter Real positionNoise[3] = {0, 0, 0};
  parameter Real velocityNoise[3] = {0, 0, 0};
  parameter Real positionQuantization[3] = {0, 0, 0};
  parameter Real velocityQuantization[3] = {0, 0, 0};
  parameter Integer seed = 1;
  ModelicaAerospace.Interfaces.Vector3Input positionECEF(each unit="m");
  ModelicaAerospace.Interfaces.Vector3Input velocityECEF(each unit="m/s");
  ModelicaAerospace.Interfaces.Vector3Output measuredPositionECEF(each unit="m");
  ModelicaAerospace.Interfaces.Vector3Output measuredVelocityECEF(each unit="m/s");
protected
  ModelicaAerospace.Sensors.VectorSensor positionSensor(
    bias=positionBias,
    noiseAmplitude=positionNoise,
    quantizationInterval=positionQuantization,
    seed=seed);
  ModelicaAerospace.Sensors.VectorSensor velocitySensor(
    bias=velocityBias,
    noiseAmplitude=velocityNoise,
    quantizationInterval=velocityQuantization,
    seed=seed + 10);
equation
  positionSensor.truth = positionECEF;
  velocitySensor.truth = velocityECEF;
  measuredPositionECEF = positionSensor.measurement;
  measuredVelocityECEF = velocitySensor.measurement;
end GPS;
