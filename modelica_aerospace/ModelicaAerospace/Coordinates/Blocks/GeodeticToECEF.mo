within ModelicaAerospace.Coordinates.Blocks;
block GeodeticToECEF "Convert geodetic latitude, longitude, and altitude to ECEF"
  ModelicaAerospace.Interfaces.RealInput latitude(unit="rad");
  ModelicaAerospace.Interfaces.RealInput longitude(unit="rad");
  ModelicaAerospace.Interfaces.RealInput altitude(unit="m");
  ModelicaAerospace.Interfaces.Vector3Output ecef(each unit="m");
protected
  ModelicaAerospace.Types.GeodeticPosition geodetic;
equation
  geodetic.latitude = latitude;
  geodetic.longitude = longitude;
  geodetic.altitude = altitude;
  ecef = ModelicaAerospace.Coordinates.geodeticToECEF(geodetic);
end GeodeticToECEF;
