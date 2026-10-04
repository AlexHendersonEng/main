within ModelicaAerospace.Environment.Gravity;
function centrifugalAcceleration "Calculate outward centrifugal acceleration in ECEF"
  input ModelicaAerospace.Types.Length positionECEF[3];
  output ModelicaAerospace.Types.Acceleration accelerationECEF[3];
protected
  ModelicaAerospace.Types.AngularVelocity earthRate[3];
algorithm
  earthRate := {0, 0, ModelicaAerospace.Constants.WGS84.earthRotationRate};
  accelerationECEF := -ModelicaAerospace.Mathematics.cross3(
    earthRate,
    ModelicaAerospace.Mathematics.cross3(earthRate, positionECEF));
end centrifugalAcceleration;
