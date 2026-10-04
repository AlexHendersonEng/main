within ModelicaAerospace.Coordinates;
function ecefToNEDMatrix "Rotation matrix from ECEF components to local NED"
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
    -sinLatitude * cosLongitude, -sinLatitude * sinLongitude, cosLatitude;
    -sinLongitude, cosLongitude, 0;
    -cosLatitude * cosLongitude, -cosLatitude * sinLongitude, -sinLatitude];
end ecefToNEDMatrix;
