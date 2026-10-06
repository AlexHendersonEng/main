within ModelicaMaritime.Environment.Water.Blocks;
block ProfileWater "Tabulated temperature, salinity, density, pressure, and current profile"
  extends ModelicaMaritime.Interfaces.PartialEnvironment;
  parameter Integer nDepths(min=2) = 3;
  parameter ModelicaMaritime.Types.Length depthGrid[nDepths] = {0, 50, 100};
  parameter ModelicaMaritime.Types.Temperature temperatureTable[nDepths] =
    {290, 285, 283};
  parameter Real salinityTable[nDepths](each unit="kg/kg", each min=0) =
    {0.034, 0.035, 0.036};
  parameter ModelicaMaritime.Types.Velocity currentTable[nDepths, 3] =
    [0, 0, 0; 0, 0, 0; 0, 0, 0];
  parameter ModelicaMaritime.Types.Temperature referenceTemperature = 288.15;
  parameter Real referenceSalinity(unit="kg/kg", min=0) = 0.035;
  parameter ModelicaMaritime.Types.Density referenceDensity =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter Real thermalExpansion(unit="1/K", min=0) = 2e-4;
  parameter Real halineContraction(min=0) = 0.8;
  parameter ModelicaMaritime.Types.Pressure surfacePressure =
    ModelicaMaritime.Constants.standardAtmosphericPressure;
  output Real salinity(unit="kg/kg");
protected
  Real densityTable[nDepths](each unit="kg/m3");
equation
  assert(depth >= 0, "ProfileWater requires non-negative depth");
  for index in 1:nDepths loop
    densityTable[index] = referenceDensity * (
      1
      - thermalExpansion * (temperatureTable[index] - referenceTemperature)
      + halineContraction * (salinityTable[index] - referenceSalinity));
    assert(temperatureTable[index] > 0, "Water profile temperature must be positive");
    assert(densityTable[index] > 0, "Water profile density must be positive");
  end for;
  temperature = ModelicaMaritime.Environment.interpolateTable(
    depth,
    depthGrid,
    temperatureTable);
  salinity = ModelicaMaritime.Environment.interpolateTable(
    depth,
    depthGrid,
    salinityTable);
  density = ModelicaMaritime.Environment.interpolateTable(
    depth,
    depthGrid,
    densityTable);
  pressure = surfacePressure
    + ModelicaMaritime.Constants.standardGravity
      * ModelicaMaritime.Environment.integrateTable(depth, depthGrid, densityTable);
  currentVelocityNED =
    ModelicaMaritime.Environment.Current.tabulatedCurrentProfile(
      depth,
      depthGrid,
      currentTable);
end ProfileWater;
