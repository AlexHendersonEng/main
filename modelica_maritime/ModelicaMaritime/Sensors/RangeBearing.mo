within ModelicaMaritime.Sensors;
block RangeBearing "Horizontal NED range and bearing measurement"
  parameter Real rangeBias(unit="m") = 0;
  parameter Real bearingBias(unit="rad") = 0;
  parameter Real rangeNoise(unit="m") = 0;
  parameter Real bearingNoise(unit="rad") = 0;
  parameter Integer seed = 1;
  ModelicaMaritime.Interfaces.Vector3Input observerPositionNED(each unit="m");
  ModelicaMaritime.Interfaces.Vector3Input targetPositionNED(each unit="m");
  ModelicaMaritime.Interfaces.RealOutput measuredRange(unit="m");
  ModelicaMaritime.Interfaces.RealOutput measuredBearing(unit="rad");
protected
  Real north;
  Real east;
  ModelicaMaritime.Sensors.ScalarSensor rangeSensor(
    bias=rangeBias,
    noiseAmplitude=rangeNoise,
    seed=seed);
  ModelicaMaritime.Sensors.ScalarSensor bearingSensor(
    bias=bearingBias,
    noiseAmplitude=bearingNoise,
    seed=seed + 1);
equation
  north = targetPositionNED[1] - observerPositionNED[1];
  east = targetPositionNED[2] - observerPositionNED[2];
  rangeSensor.truth = sqrt(north * north + east * east);
  bearingSensor.truth = atan2(east, north);
  measuredRange = rangeSensor.measurement;
  measuredBearing =
    ModelicaMaritime.Mathematics.wrapHeading(bearingSensor.measurement);
end RangeBearing;
