within ModelicaAerospace.Environment.Gravity;
function earthRotationVelocity "Calculate Earth-rotation transport velocity in ECEF"
  input ModelicaAerospace.Types.Length positionECEF[3];
  output ModelicaAerospace.Types.Velocity velocityECEF[3];
algorithm
  velocityECEF := ModelicaAerospace.Mathematics.cross3(
    {0, 0, ModelicaAerospace.Constants.WGS84.earthRotationRate},
    positionECEF);
end earthRotationVelocity;
