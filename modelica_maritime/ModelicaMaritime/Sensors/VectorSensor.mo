within ModelicaMaritime.Sensors;
block VectorSensor "Biased, deterministically noisy, and quantized vector sensor"
  parameter Real bias[3] = {0, 0, 0};
  parameter Real noiseAmplitude[3] = {0, 0, 0};
  parameter Real quantizationInterval[3] = {0, 0, 0};
  parameter Integer seed = 1;
  ModelicaMaritime.Interfaces.Vector3Input truth;
  ModelicaMaritime.Interfaces.Vector3Output measurement;
protected
  Real rawMeasurement[3];
equation
  for axis in 1:3 loop
    rawMeasurement[axis] = truth[axis] + bias[axis] + noiseAmplitude[axis]
      * ModelicaMaritime.Sensors.deterministicNoise(time, seed, axis);
    measurement[axis] = ModelicaMaritime.Sensors.quantize(
      rawMeasurement[axis],
      quantizationInterval[axis]);
  end for;
end VectorSensor;
