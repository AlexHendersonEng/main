within ModelicaAerospace.Coordinates.Blocks;
block ECEFToGeodetic "Convert ECEF coordinates to geodetic outputs"
  ModelicaAerospace.Interfaces.Vector3Input ecef(each unit="m");
  ModelicaAerospace.Interfaces.RealOutput latitude(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput longitude(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput altitude(unit="m");
protected
  ModelicaAerospace.Types.GeodeticPosition geodetic;
equation
  geodetic = ModelicaAerospace.Coordinates.ecefToGeodetic(ecef);
  latitude = geodetic.latitude;
  longitude = geodetic.longitude;
  altitude = geodetic.altitude;
end ECEFToGeodetic;
