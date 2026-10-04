within ModelicaAerospace.Sensors;
block VectorSensor "Biased, noisy, and quantized three-axis sensor"
  extends ModelicaAerospace.Interfaces.PartialSensor;
  parameter Real bias[3] = {0, 0, 0};
  parameter Real noiseAmplitude[3] = {0, 0, 0};
  parameter Real quantizationInterval[3] = {0, 0, 0};
  parameter Integer seed = 1;
protected
  Real rawMeasurement[3];
equation
  for index in 1:3 loop
    rawMeasurement[index] = truth[index] + bias[index]
      + noiseAmplitude[index]
      * ModelicaAerospace.Environment.Wind.deterministicNoise(time, seed, index);
    measurement[index] = ModelicaAerospace.Sensors.quantize(
      rawMeasurement[index],
      quantizationInterval[index]);
  end for;
end VectorSensor;
