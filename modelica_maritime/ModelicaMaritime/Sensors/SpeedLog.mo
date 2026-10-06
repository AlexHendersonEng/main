within ModelicaMaritime.Sensors;
block SpeedLog "Water-relative body speed and surge log"
  parameter Real speedBias(unit="m/s") = 0;
  parameter Real surgeBias(unit="m/s") = 0;
  parameter Real noiseAmplitude(unit="m/s") = 0;
  parameter Real quantizationInterval(unit="m/s") = 0;
  parameter Integer seed = 1;
  ModelicaMaritime.Interfaces.Vector3Input relativeVelocityBody(each unit="m/s");
  ModelicaMaritime.Interfaces.RealOutput measuredSpeed(unit="m/s");
  ModelicaMaritime.Interfaces.RealOutput measuredSurge(unit="m/s");
protected
  ModelicaMaritime.Sensors.ScalarSensor speedSensor(
    bias=speedBias,
    noiseAmplitude=noiseAmplitude,
    quantizationInterval=quantizationInterval,
    seed=seed);
  ModelicaMaritime.Sensors.ScalarSensor surgeSensor(
    bias=surgeBias,
    noiseAmplitude=noiseAmplitude,
    quantizationInterval=quantizationInterval,
    seed=seed + 1);
equation
  speedSensor.truth = sqrt(relativeVelocityBody * relativeVelocityBody);
  surgeSensor.truth = relativeVelocityBody[1];
  measuredSpeed = speedSensor.measurement;
  measuredSurge = surgeSensor.measurement;
end SpeedLog;
