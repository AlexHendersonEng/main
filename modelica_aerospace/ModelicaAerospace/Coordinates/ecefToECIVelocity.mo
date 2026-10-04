within ModelicaAerospace.Coordinates;
function ecefToECIVelocity "Convert ECEF position and velocity to inertial velocity"
  input ModelicaAerospace.Types.Length positionECEF[3];
  input ModelicaAerospace.Types.Velocity velocityECEF[3];
  input Real elapsedTime(unit="s");
  input ModelicaAerospace.Types.Angle angleAtEpoch = 0;
  output ModelicaAerospace.Types.Velocity velocityECI[3];
protected
  ModelicaAerospace.Types.AngularVelocity earthRate[3];
  ModelicaAerospace.Types.Velocity transportVelocity[3];
algorithm
  earthRate := {0, 0, ModelicaAerospace.Constants.WGS84.earthRotationRate};
  transportVelocity := ModelicaAerospace.Mathematics.cross3(earthRate, positionECEF);
  velocityECI := ModelicaAerospace.Coordinates.ecefToECIVector(
    velocityECEF + transportVelocity,
    elapsedTime,
    angleAtEpoch);
end ecefToECIVelocity;
