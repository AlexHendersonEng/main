within ModelicaAerospace.Examples;
model AtmosphereGeodesy "Atmosphere and WGS-84 coordinate demonstration"
  parameter ModelicaAerospace.Types.Length altitude = 10000;
  parameter ModelicaAerospace.Types.Angle latitude = 0.7853981633974483;
  parameter ModelicaAerospace.Types.Angle longitude = -1.623156204354726;
  ModelicaAerospace.Environment.Atmosphere.Blocks.StandardAtmosphere atmosphere;
  output Real temperature(unit="K");
  output Real pressure(unit="Pa");
  output Real density(unit="kg/m3");
  output Real speedOfSound(unit="m/s");
  output Real positionECEF[3](each unit="m");
  output Real recoveredGeodetic[3] "{latitude, longitude, altitude}";
protected
  ModelicaAerospace.Types.GeodeticPosition location(
    latitude=latitude,
    longitude=longitude,
    altitude=altitude);
  ModelicaAerospace.Types.GeodeticPosition recovered;
equation
  atmosphere.altitude = altitude + 10 * time;
  temperature = atmosphere.temperature;
  pressure = atmosphere.pressure;
  density = atmosphere.density;
  speedOfSound = atmosphere.speedOfSound;
  positionECEF = ModelicaAerospace.Coordinates.geodeticToECEF(location);
  recovered = ModelicaAerospace.Coordinates.ecefToGeodetic(positionECEF);
  recoveredGeodetic = {
    recovered.latitude,
    recovered.longitude,
    recovered.altitude};
  annotation (
    experiment(StartTime=0, StopTime=0.1, Tolerance=1e-9, Interval=0.1),
    Documentation(info="<html>
<p>Evaluates the U.S. Standard Atmosphere 1976 over a one-metre ascent from
10 km geometric altitude and round-trips a 45 degree north, 93 degree west
WGS-84 location through ECEF coordinates. Expected outputs are
standard-atmosphere properties and a geodetic round trip accurate to
numerical precision.</p>
</html>"));
end AtmosphereGeodesy;
