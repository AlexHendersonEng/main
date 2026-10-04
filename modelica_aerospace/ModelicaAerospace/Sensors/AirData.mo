within ModelicaAerospace.Sensors;
block AirData "Configurable deterministic air-data sensor"
  parameter Real bias[4] = {0, 0, 0, 0};
  parameter Real noiseAmplitude[4] = {0, 0, 0, 0};
  parameter Real quantizationInterval[4] = {0, 0, 0, 0};
  parameter Integer seed = 1;
  ModelicaAerospace.Interfaces.RealInput trueAirspeed(unit="m/s");
  ModelicaAerospace.Interfaces.RealInput angleOfAttack(unit="rad");
  ModelicaAerospace.Interfaces.RealInput sideslip(unit="rad");
  ModelicaAerospace.Interfaces.RealInput altitude(unit="m");
  ModelicaAerospace.Interfaces.RealOutput measuredAirspeed(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput measuredAngleOfAttack(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput measuredSideslip(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput measuredAltitude(unit="m");
protected
  ModelicaAerospace.Sensors.ScalarSensor airspeedSensor(
    bias=bias[1],
    noiseAmplitude=noiseAmplitude[1],
    quantizationInterval=quantizationInterval[1],
    seed=seed);
  ModelicaAerospace.Sensors.ScalarSensor angleSensor(
    bias=bias[2],
    noiseAmplitude=noiseAmplitude[2],
    quantizationInterval=quantizationInterval[2],
    seed=seed + 1);
  ModelicaAerospace.Sensors.ScalarSensor sideslipSensor(
    bias=bias[3],
    noiseAmplitude=noiseAmplitude[3],
    quantizationInterval=quantizationInterval[3],
    seed=seed + 2);
  ModelicaAerospace.Sensors.ScalarSensor altitudeSensor(
    bias=bias[4],
    noiseAmplitude=noiseAmplitude[4],
    quantizationInterval=quantizationInterval[4],
    seed=seed + 3);
equation
  airspeedSensor.truth = trueAirspeed;
  angleSensor.truth = angleOfAttack;
  sideslipSensor.truth = sideslip;
  altitudeSensor.truth = altitude;
  measuredAirspeed = airspeedSensor.measurement;
  measuredAngleOfAttack = angleSensor.measurement;
  measuredSideslip = sideslipSensor.measurement;
  measuredAltitude = altitudeSensor.measurement;
end AirData;
