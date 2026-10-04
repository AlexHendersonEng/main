within ModelicaAerospace.Coordinates;
function ecefToECIAcceleration
  "Convert ECEF position, velocity, and acceleration to inertial acceleration"
  input ModelicaAerospace.Types.Length positionECEF[3];
  input ModelicaAerospace.Types.Velocity velocityECEF[3];
  input ModelicaAerospace.Types.Acceleration accelerationECEF[3];
  input Real elapsedTime(unit="s");
  input ModelicaAerospace.Types.Angle angleAtEpoch = 0;
  output ModelicaAerospace.Types.Acceleration accelerationECI[3];
protected
  ModelicaAerospace.Types.AngularVelocity earthRate[3];
  ModelicaAerospace.Types.Acceleration coriolis[3];
  ModelicaAerospace.Types.Acceleration centripetal[3];
algorithm
  earthRate := {0, 0, ModelicaAerospace.Constants.WGS84.earthRotationRate};
  coriolis := 2 * ModelicaAerospace.Mathematics.cross3(earthRate, velocityECEF);
  centripetal := ModelicaAerospace.Mathematics.cross3(
    earthRate,
    ModelicaAerospace.Mathematics.cross3(earthRate, positionECEF));
  accelerationECI := ModelicaAerospace.Coordinates.ecefToECIVector(
    accelerationECEF + coriolis + centripetal,
    elapsedTime,
    angleAtEpoch);
end ecefToECIAcceleration;
