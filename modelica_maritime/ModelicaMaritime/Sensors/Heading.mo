within ModelicaMaritime.Sensors;
block Heading "Wrapped heading sensor"
  parameter Real bias(unit="rad") = 0;
  parameter Real noiseAmplitude(unit="rad") = 0;
  parameter Real quantizationInterval(unit="rad") = 0;
  parameter Integer seed = 1;
  ModelicaMaritime.Interfaces.RealInput heading(unit="rad");
  ModelicaMaritime.Interfaces.RealOutput measuredHeading(unit="rad");
protected
  ModelicaMaritime.Sensors.ScalarSensor sensor(
    bias=bias,
    noiseAmplitude=noiseAmplitude,
    quantizationInterval=quantizationInterval,
    seed=seed);
equation
  sensor.truth = heading;
  measuredHeading = ModelicaMaritime.Mathematics.wrapHeading(sensor.measurement);
end Heading;
