within ModelicaMaritime.Types;
record WaterState "Local water-column state"
  ModelicaMaritime.Types.Temperature temperature = 288.15;
  ModelicaMaritime.Types.Pressure pressure =
    ModelicaMaritime.Constants.standardAtmosphericPressure;
  ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  Real salinity(unit="kg/kg", min=0) = 0.035 "Mass fraction of dissolved salts";
  ModelicaMaritime.Types.Velocity currentVelocityNED[3] = {0, 0, 0};
end WaterState;
