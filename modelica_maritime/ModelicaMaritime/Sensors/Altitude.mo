within ModelicaMaritime.Sensors;
block Altitude "Seafloor-altitude sensor"
  parameter Real bias(unit="m") = 0;
  parameter Real noiseAmplitude(unit="m") = 0;
  parameter Real quantizationInterval(unit="m") = 0;
  parameter Integer seed = 1;
  ModelicaMaritime.Interfaces.RealInput vehicleDepth(unit="m");
  ModelicaMaritime.Interfaces.RealInput seafloorDepth(unit="m");
  ModelicaMaritime.Interfaces.RealOutput measuredAltitude(unit="m");
protected
  ModelicaMaritime.Sensors.ScalarSensor sensor(
    bias=bias,
    noiseAmplitude=noiseAmplitude,
    quantizationInterval=quantizationInterval,
    seed=seed);
equation
  sensor.truth = ModelicaMaritime.Coordinates.altitudeAboveSeafloor(
    vehicleDepth,
    seafloorDepth);
  measuredAltitude = sensor.measurement;
end Altitude;
