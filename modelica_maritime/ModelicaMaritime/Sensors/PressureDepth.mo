within ModelicaMaritime.Sensors;
block PressureDepth "Pressure-derived positive-down depth sensor"
  parameter ModelicaMaritime.Types.Density waterDensity =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter ModelicaMaritime.Types.Pressure surfacePressure =
    ModelicaMaritime.Constants.standardAtmosphericPressure;
  parameter Real pressureBias(unit="Pa") = 0;
  parameter Real pressureNoise(unit="Pa") = 0;
  parameter Real pressureQuantization(unit="Pa") = 0;
  parameter Integer seed = 1;
  ModelicaMaritime.Interfaces.RealInput depth(unit="m");
  ModelicaMaritime.Interfaces.RealOutput measuredPressure(unit="Pa");
  ModelicaMaritime.Interfaces.RealOutput measuredDepth(unit="m");
protected
  ModelicaMaritime.Sensors.ScalarSensor pressureSensor(
    bias=pressureBias,
    noiseAmplitude=pressureNoise,
    quantizationInterval=pressureQuantization,
    seed=seed);
equation
  assert(waterDensity > 0, "Depth-sensor water density must be positive");
  pressureSensor.truth = surfacePressure
    + waterDensity * ModelicaMaritime.Constants.standardGravity * depth;
  measuredPressure = pressureSensor.measurement;
  measuredDepth = (measuredPressure - surfacePressure)
    / (waterDensity * ModelicaMaritime.Constants.standardGravity);
end PressureDepth;
