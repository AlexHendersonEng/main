within ModelicaAerospace.Coordinates;
function ecefToGeodetic "Convert ECEF coordinates to WGS-84 geodetic position"
  input ModelicaAerospace.Types.Length ecef[3];
  output ModelicaAerospace.Types.GeodeticPosition geodetic;
protected
  Real horizontal;
  Real theta;
  Real sinTheta;
  Real cosTheta;
  Real secondEccentricitySquared;
  ModelicaAerospace.Types.Length radius;
algorithm
  horizontal := sqrt(ecef[1] * ecef[1] + ecef[2] * ecef[2]);
  assert(
    horizontal + abs(ecef[3]) > ModelicaAerospace.Constants.Numerics.small,
    "Geodetic position is undefined at the Earth center");
  if horizontal <= 1e-6 then
    geodetic.latitude := if ecef[3] >= 0 then 1.570796326794897 else -1.570796326794897;
    geodetic.longitude := 0;
    geodetic.altitude :=
      abs(ecef[3]) - ModelicaAerospace.Constants.WGS84.semiMinorAxis;
  else
    geodetic.longitude := atan2(ecef[2], ecef[1]);
    secondEccentricitySquared :=
      (
        ModelicaAerospace.Constants.WGS84.semiMajorAxis
        * ModelicaAerospace.Constants.WGS84.semiMajorAxis
        - ModelicaAerospace.Constants.WGS84.semiMinorAxis
        * ModelicaAerospace.Constants.WGS84.semiMinorAxis
      ) / (
        ModelicaAerospace.Constants.WGS84.semiMinorAxis
        * ModelicaAerospace.Constants.WGS84.semiMinorAxis
      );
    theta := atan2(
      ecef[3] * ModelicaAerospace.Constants.WGS84.semiMajorAxis,
      horizontal * ModelicaAerospace.Constants.WGS84.semiMinorAxis);
    sinTheta := sin(theta);
    cosTheta := cos(theta);
    geodetic.latitude := atan2(
      ecef[3]
      + secondEccentricitySquared
      * ModelicaAerospace.Constants.WGS84.semiMinorAxis
      * sinTheta * sinTheta * sinTheta,
      horizontal
      - ModelicaAerospace.Constants.WGS84.firstEccentricitySquared
      * ModelicaAerospace.Constants.WGS84.semiMajorAxis
      * cosTheta * cosTheta * cosTheta);
    radius := ModelicaAerospace.Coordinates.primeVerticalRadius(geodetic.latitude);
    geodetic.altitude := horizontal / cos(geodetic.latitude) - radius;
  end if;
end ecefToGeodetic;
