within ModelicaAerospace.Sensors;
block ScalarSensor "Biased, noisy, and quantized scalar sensor"
  parameter Real bias = 0;
  parameter Real noiseAmplitude = 0;
  parameter Real quantizationInterval = 0;
  parameter Integer seed = 1;
  ModelicaAerospace.Interfaces.RealInput truth;
  ModelicaAerospace.Interfaces.RealOutput measurement;
protected
  Real rawMeasurement;
equation
  rawMeasurement = truth + bias + noiseAmplitude
    * ModelicaAerospace.Environment.Wind.deterministicNoise(time, seed, 1);
  measurement =
    ModelicaAerospace.Sensors.quantize(rawMeasurement, quantizationInterval);
end ScalarSensor;
