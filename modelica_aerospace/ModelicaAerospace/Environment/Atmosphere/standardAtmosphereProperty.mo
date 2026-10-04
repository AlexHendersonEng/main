within ModelicaAerospace.Environment.Atmosphere;
function standardAtmosphereProperty "Evaluate one U.S. Standard Atmosphere 1976 property"
  input ModelicaAerospace.Types.Length geometricAltitude;
  input Integer propertyIndex
    "1 temperature, 2 pressure, 3 density, 4 speed of sound, 5 dynamic viscosity";
  output Real value;
protected
  ModelicaAerospace.Types.Length altitude;
  ModelicaAerospace.Types.Length baseAltitude;
  ModelicaAerospace.Types.Temperature baseTemperature;
  ModelicaAerospace.Types.Pressure basePressure;
  ModelicaAerospace.Types.Temperature localTemperature;
  ModelicaAerospace.Types.Pressure localPressure;
  ModelicaAerospace.Types.Density localDensity;
  ModelicaAerospace.Types.Velocity localSpeedOfSound;
  ModelicaAerospace.Types.DynamicViscosity localDynamicViscosity;
  Real lapseRate(unit="K/m");
  constant Real gasConstant =
    ModelicaAerospace.Constants.StandardAtmosphere.specificGasConstant;
  constant ModelicaAerospace.Types.Acceleration gravity =
    ModelicaAerospace.Constants.StandardAtmosphere.standardGravity;
algorithm
  assert(
    geometricAltitude
      > -ModelicaAerospace.Constants.StandardAtmosphere.geopotentialRadius,
    "Geometric altitude must be greater than the negative geopotential Earth radius");
  altitude := ModelicaAerospace.Constants.StandardAtmosphere.geopotentialRadius
    * geometricAltitude
    / (ModelicaAerospace.Constants.StandardAtmosphere.geopotentialRadius
      + geometricAltitude);
  assert(
    altitude >= -5000 and altitude <= 84852,
    "Standard atmosphere geopotential altitude must be in [-5000, 84852] m");
  assert(
    propertyIndex >= 1 and propertyIndex <= 5,
    "Standard atmosphere property index must be in [1, 5]");

  if altitude < 11000 then
    baseAltitude := 0;
    baseTemperature := 288.15;
    basePressure := 101325;
    lapseRate := -0.0065;
  elseif altitude < 20000 then
    baseAltitude := 11000;
    baseTemperature := 216.65;
    basePressure := 22632.0400950078;
    lapseRate := 0;
  elseif altitude < 32000 then
    baseAltitude := 20000;
    baseTemperature := 216.65;
    basePressure := 5474.87742428105;
    lapseRate := 0.001;
  elseif altitude < 47000 then
    baseAltitude := 32000;
    baseTemperature := 228.65;
    basePressure := 868.015776620216;
    lapseRate := 0.0028;
  elseif altitude < 51000 then
    baseAltitude := 47000;
    baseTemperature := 270.65;
    basePressure := 110.90577336731;
    lapseRate := 0;
  elseif altitude < 71000 then
    baseAltitude := 51000;
    baseTemperature := 270.65;
    basePressure := 66.9385281211797;
    lapseRate := -0.0028;
  else
    baseAltitude := 71000;
    baseTemperature := 214.65;
    basePressure := 3.95639216039661;
    lapseRate := -0.002;
  end if;

  localTemperature := baseTemperature + lapseRate * (altitude - baseAltitude);
  if abs(lapseRate) < ModelicaAerospace.Constants.Numerics.small then
    localPressure := basePressure * exp(
      -gravity * (altitude - baseAltitude) / (gasConstant * baseTemperature));
  else
    localPressure := basePressure * (
      baseTemperature / localTemperature) ^ (gravity / (gasConstant * lapseRate));
  end if;
  localDensity := localPressure / (gasConstant * localTemperature);
  localSpeedOfSound := sqrt(
    ModelicaAerospace.Constants.StandardAtmosphere.heatCapacityRatio
    * gasConstant * localTemperature);
  localDynamicViscosity := 1.716e-5 * (localTemperature / 273.15) ^ 1.5
    * (273.15 + 110.4) / (localTemperature + 110.4);

  if propertyIndex == 1 then
    value := localTemperature;
  elseif propertyIndex == 2 then
    value := localPressure;
  elseif propertyIndex == 3 then
    value := localDensity;
  elseif propertyIndex == 4 then
    value := localSpeedOfSound;
  else
    value := localDynamicViscosity;
  end if;
end standardAtmosphereProperty;
