within ModelicaAerospace.Environment.Gravity;
function normalGravity "Calculate WGS-84 normal gravity magnitude"
  input ModelicaAerospace.Types.Angle latitude;
  input ModelicaAerospace.Types.Length altitude = 0;
  output ModelicaAerospace.Types.Acceleration gravity;
protected
  constant ModelicaAerospace.Types.Acceleration equatorialGravity = 9.7803253359;
  constant Real somigliana = 0.00193185265241;
  Real sinLatitude;
  ModelicaAerospace.Types.Acceleration surfaceGravity;
algorithm
  assert(
    latitude >= -1.570796326794897 and latitude <= 1.570796326794897,
    "Latitude must be within [-pi/2, pi/2]");
  assert(
    ModelicaAerospace.Constants.WGS84.semiMajorAxis + altitude > 0,
    "Altitude must remain above the Earth center");
  sinLatitude := sin(latitude);
  surfaceGravity := equatorialGravity
    * (1 + somigliana * sinLatitude * sinLatitude)
    / sqrt(
      1 - ModelicaAerospace.Constants.WGS84.firstEccentricitySquared
      * sinLatitude * sinLatitude);
  gravity := surfaceGravity * (
    ModelicaAerospace.Constants.WGS84.semiMajorAxis
    / (ModelicaAerospace.Constants.WGS84.semiMajorAxis + altitude)) ^ 2;
end normalGravity;
