within ModelicaAerospace.Coordinates;
function eciToECEFVelocity "Convert ECI position and velocity to rotating-Earth velocity"
  input ModelicaAerospace.Types.Length positionECI[3];
  input ModelicaAerospace.Types.Velocity velocityECI[3];
  input Real elapsedTime(unit="s");
  input ModelicaAerospace.Types.Angle angleAtEpoch = 0;
  output ModelicaAerospace.Types.Velocity velocityECEF[3];
protected
  ModelicaAerospace.Types.Length positionECEF[3];
  ModelicaAerospace.Types.Velocity rotatedVelocity[3];
  ModelicaAerospace.Types.AngularVelocity earthRate[3];
algorithm
  positionECEF := ModelicaAerospace.Coordinates.eciToECEFVector(
    positionECI,
    elapsedTime,
    angleAtEpoch);
  rotatedVelocity := ModelicaAerospace.Coordinates.eciToECEFVector(
    velocityECI,
    elapsedTime,
    angleAtEpoch);
  earthRate := {0, 0, ModelicaAerospace.Constants.WGS84.earthRotationRate};
  velocityECEF :=
    rotatedVelocity - ModelicaAerospace.Mathematics.cross3(earthRate, positionECEF);
end eciToECEFVelocity;
