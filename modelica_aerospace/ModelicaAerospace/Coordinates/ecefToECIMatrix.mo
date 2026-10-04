within ModelicaAerospace.Coordinates;
function ecefToECIMatrix "Earth rotation from ECEF to ECI at time"
  input Real elapsedTime(unit="s");
  input ModelicaAerospace.Types.Angle angleAtEpoch = 0;
  output ModelicaAerospace.Types.Matrix3 dcm;
protected
  ModelicaAerospace.Types.Angle angle;
algorithm
  angle :=
    angleAtEpoch + ModelicaAerospace.Constants.WGS84.earthRotationRate * elapsedTime;
  dcm := [cos(angle), -sin(angle), 0; sin(angle), cos(angle), 0; 0, 0, 1];
end ecefToECIMatrix;
