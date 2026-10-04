within ModelicaAerospace.Coordinates;
function eciToECEFAcceleration
  "Convert ECI position, velocity, and acceleration to rotating-Earth acceleration"
  input ModelicaAerospace.Types.Length positionECI[3];
  input ModelicaAerospace.Types.Velocity velocityECI[3];
  input ModelicaAerospace.Types.Acceleration accelerationECI[3];
  input Real elapsedTime(unit="s");
  input ModelicaAerospace.Types.Angle angleAtEpoch = 0;
  output ModelicaAerospace.Types.Acceleration accelerationECEF[3];
protected
  ModelicaAerospace.Types.Length positionECEF[3];
  ModelicaAerospace.Types.Velocity velocityECEF[3];
  ModelicaAerospace.Types.Acceleration rotatedAcceleration[3];
  ModelicaAerospace.Types.AngularVelocity earthRate[3];
algorithm
  positionECEF := ModelicaAerospace.Coordinates.eciToECEFVector(
    positionECI,
    elapsedTime,
    angleAtEpoch);
  velocityECEF := ModelicaAerospace.Coordinates.eciToECEFVelocity(
    positionECI,
    velocityECI,
    elapsedTime,
    angleAtEpoch);
  rotatedAcceleration := ModelicaAerospace.Coordinates.eciToECEFVector(
    accelerationECI,
    elapsedTime,
    angleAtEpoch);
  earthRate := {0, 0, ModelicaAerospace.Constants.WGS84.earthRotationRate};
  accelerationECEF :=
    rotatedAcceleration
    - 2 * ModelicaAerospace.Mathematics.cross3(earthRate, velocityECEF)
    - ModelicaAerospace.Mathematics.cross3(
      earthRate,
      ModelicaAerospace.Mathematics.cross3(earthRate, positionECEF));
end eciToECEFAcceleration;
