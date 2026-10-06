within ModelicaMaritime.Environment.Water;
function densityLinear "Linearized seawater density from temperature and salinity"
  input ModelicaMaritime.Types.Temperature temperature;
  input Real salinity(unit="kg/kg", min=0);
  input ModelicaMaritime.Types.Density referenceDensity =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  input ModelicaMaritime.Types.Temperature referenceTemperature = 288.15;
  input Real referenceSalinity(unit="kg/kg", min=0) = 0.035;
  input Real thermalExpansion(unit="1/K", min=0) = 2e-4;
  input Real halineContraction(min=0) = 0.8;
  output ModelicaMaritime.Types.Density density;
algorithm
  assert(referenceDensity > 0, "Reference water density must be positive");
  assert(temperature > 0, "Water temperature must be positive");
  density := referenceDensity * (
    1
    - thermalExpansion * (temperature - referenceTemperature)
    + halineContraction * (salinity - referenceSalinity));
  assert(density > 0, "Linear water-density model produced non-positive density");
end densityLinear;
