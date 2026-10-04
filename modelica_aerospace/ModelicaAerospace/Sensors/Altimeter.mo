within ModelicaAerospace.Sensors;
block Altimeter "Configurable deterministic altitude sensor"
  parameter Real bias(unit="m") = 0;
  parameter Real noiseAmplitude(unit="m") = 0;
  parameter Real quantizationInterval(unit="m") = 0;
  parameter Integer seed = 1;
  ModelicaAerospace.Interfaces.RealInput altitude(unit="m");
  ModelicaAerospace.Interfaces.RealOutput measuredAltitude(unit="m");
protected
  ModelicaAerospace.Sensors.ScalarSensor sensor(
    bias=bias,
    noiseAmplitude=noiseAmplitude,
    quantizationInterval=quantizationInterval,
    seed=seed);
equation
  sensor.truth = altitude;
  measuredAltitude = sensor.measurement;
end Altimeter;
