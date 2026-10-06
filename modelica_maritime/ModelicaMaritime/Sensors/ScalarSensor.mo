within ModelicaMaritime.Sensors;
block ScalarSensor "Biased, deterministically noisy, and quantized scalar sensor"
  extends ModelicaMaritime.Interfaces.PartialSensor;
  parameter Real bias = 0;
  parameter Real noiseAmplitude = 0;
  parameter Real quantizationInterval = 0;
  parameter Integer seed = 1;
protected
  Real rawMeasurement;
equation
  rawMeasurement = truth + bias + noiseAmplitude
    * ModelicaMaritime.Sensors.deterministicNoise(time, seed, 1);
  measurement = ModelicaMaritime.Sensors.quantize(
    rawMeasurement,
    quantizationInterval);
end ScalarSensor;
