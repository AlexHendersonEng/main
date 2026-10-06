within ModelicaMaritime.Tests.Environment;
model ProfileValidation "Validate tabulated water/current, wind, and bathymetry"
  ModelicaMaritime.Environment.Water.Blocks.ProfileWater water(
    nDepths=3,
    depthGrid={0, 50, 100},
    temperatureTable={290, 285, 283},
    salinityTable={0.034, 0.035, 0.036},
    currentTable=[1, 0, 0; 0.5, 0.2, 0; 0.2, 0.3, 0.1],
    referenceTemperature=288,
    referenceSalinity=0.035,
    referenceDensity=1025);
  ModelicaMaritime.Environment.Current.Blocks.TabulatedCurrent current(
    nDepths=3,
    depthGrid={0, 50, 100},
    currentTable=[1, 0, 0; 0.5, 0.2, 0; 0.2, 0.3, 0.1]);
  ModelicaMaritime.Environment.Wind.Blocks.GustingWind wind(
    meanVelocityNED={8, -1, 0},
    gustAmplitudeNED={2, 1, 0.5},
    gustFrequency=0.25,
    phase=0.2);
  ModelicaMaritime.Environment.Wind.Blocks.SteadyWind steadyWind(
    windNED={4, 3, -0.2});
  ModelicaMaritime.Environment.Bathymetry.FlatSeafloor flat(seafloorDepth=120);
  ModelicaMaritime.Environment.Bathymetry.SlopedSeafloor slope(
    referenceDepth=100,
    slopeNorth=0.1,
    slopeEast=-0.05);
  output Real temperature;
  output Real salinity;
  output Real density;
  output Real pressure;
  output Real waterCurrent[3];
  output Real profileCurrent[3];
  output Real windVelocity[3];
  output Real directionalWind[3];
  output Real steadyWindVelocity[3];
  output Real flatAltitude;
  output Real slopeDepth;
  output Real slopeAltitude;
protected
  Real probe;
equation
  probe = 1e-7 * time;
  water.depth = 75;
  current.depth = 75;
  flat.positionNED = {200, 40, 70};
  slope.positionNED = {200, 40, 70};
  temperature = water.temperature;
  salinity = water.salinity;
  density = water.density;
  pressure = water.pressure;
  waterCurrent = water.currentVelocityNED;
  profileCurrent = current.velocityNED;
  windVelocity = wind.velocityNED;
  directionalWind = ModelicaMaritime.Environment.Wind.windVelocityNED(
    10,
    0.5235987755982989,
    -0.2);
  steadyWindVelocity = steadyWind.velocityNED;
  flatAltitude = flat.altitude + probe;
  slopeDepth = slope.depth + 2 * probe;
  slopeAltitude = slope.altitude + 3 * probe;
end ProfileValidation;
