within ModelicaMaritime.Environment.Water.Blocks;
block ConstantWater "Constant-property water column with hydrostatic pressure"
  extends ModelicaMaritime.Interfaces.PartialEnvironment;
  parameter ModelicaMaritime.Types.Temperature waterTemperature = 288.15;
  parameter ModelicaMaritime.Types.Density waterDensity =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter ModelicaMaritime.Types.Pressure surfacePressure =
    ModelicaMaritime.Constants.standardAtmosphericPressure;
  parameter ModelicaMaritime.Types.Velocity currentNED[3] = {0, 0, 0};
equation
  assert(depth >= 0, "ConstantWater requires non-negative depth");
  assert(waterDensity > 0, "ConstantWater density must be positive");
  temperature = waterTemperature;
  density = waterDensity;
  pressure = surfacePressure
    + waterDensity * ModelicaMaritime.Constants.standardGravity * depth;
  currentVelocityNED = currentNED;
end ConstantWater;
