within ModelicaAerospace.Coordinates;
function nedToECEFMatrix "Rotation matrix from local NED components to ECEF"
  input ModelicaAerospace.Types.Angle latitude;
  input ModelicaAerospace.Types.Angle longitude;
  output ModelicaAerospace.Types.Matrix3 dcm;
protected
  Real sinLatitude;
  Real cosLatitude;
  Real sinLongitude;
  Real cosLongitude;
algorithm
  sinLatitude := sin(latitude);
  cosLatitude := cos(latitude);
  sinLongitude := sin(longitude);
  cosLongitude := cos(longitude);
  dcm := [
    -sinLatitude * cosLongitude, -sinLongitude, -cosLatitude * cosLongitude;
    -sinLatitude * sinLongitude, cosLongitude, -cosLatitude * sinLongitude;
    cosLatitude, 0, -sinLatitude];
end nedToECEFMatrix;
