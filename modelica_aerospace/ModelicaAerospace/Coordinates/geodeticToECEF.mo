within ModelicaAerospace.Coordinates;
function geodeticToECEF "Convert WGS-84 geodetic position to ECEF coordinates"
  input ModelicaAerospace.Types.GeodeticPosition geodetic;
  output ModelicaAerospace.Types.Length ecef[3];
protected
  ModelicaAerospace.Types.Length radius;
  Real sinLatitude;
  Real cosLatitude;
  Real sinLongitude;
  Real cosLongitude;
algorithm
  radius := ModelicaAerospace.Coordinates.primeVerticalRadius(geodetic.latitude);
  sinLatitude := sin(geodetic.latitude);
  cosLatitude := cos(geodetic.latitude);
  sinLongitude := sin(geodetic.longitude);
  cosLongitude := cos(geodetic.longitude);
  ecef[1] := (radius + geodetic.altitude) * cosLatitude * cosLongitude;
  ecef[2] := (radius + geodetic.altitude) * cosLatitude * sinLongitude;
  ecef[3] :=
    (radius * (1 - ModelicaAerospace.Constants.WGS84.firstEccentricitySquared)
    + geodetic.altitude) * sinLatitude;
end geodeticToECEF;
