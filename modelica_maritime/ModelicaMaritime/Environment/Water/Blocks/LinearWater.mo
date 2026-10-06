within ModelicaMaritime.Environment.Water.Blocks;
block LinearWater "Depth-varying temperature, salinity, density, and current"
  extends ModelicaMaritime.Interfaces.PartialEnvironment;
  parameter ModelicaMaritime.Types.Temperature surfaceTemperature = 288.15;
  parameter Real temperatureGradient(unit="K/m") = -0.01;
  parameter Real surfaceSalinity(unit="kg/kg", min=0) = 0.035;
  parameter Real salinityGradient(unit="kg/(kg.m)") = 1e-6;
  parameter ModelicaMaritime.Types.Density referenceDensity =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter Real thermalExpansion(unit="1/K", min=0) = 2e-4;
  parameter Real halineContraction(min=0) = 0.8;
  parameter ModelicaMaritime.Types.Pressure surfacePressure =
    ModelicaMaritime.Constants.standardAtmosphericPressure;
  parameter ModelicaMaritime.Types.Velocity surfaceCurrentNED[3] = {0, 0, 0};
  parameter Real currentGradientNED[3](each unit="1/s") = {0, 0, 0};
protected
  Real salinity(unit="kg/kg");
  Real relativeDensityGradient(unit="1/m");
equation
  assert(depth >= 0, "LinearWater requires non-negative depth");
  temperature = surfaceTemperature + temperatureGradient * depth;
  salinity = surfaceSalinity + salinityGradient * depth;
  density = referenceDensity * (
    1
    - thermalExpansion * (temperature - surfaceTemperature)
    + halineContraction * (salinity - surfaceSalinity));
  relativeDensityGradient =
    -thermalExpansion * temperatureGradient
    + halineContraction * salinityGradient;
  assert(temperature > 0, "LinearWater produced non-positive temperature");
  assert(salinity >= 0, "LinearWater produced negative salinity");
  assert(density > 0, "LinearWater produced non-positive density");
  pressure = surfacePressure
    + referenceDensity * ModelicaMaritime.Constants.standardGravity
      * (depth + 0.5 * relativeDensityGradient * depth * depth);
  currentVelocityNED = surfaceCurrentNED + currentGradientNED * depth;
end LinearWater;
