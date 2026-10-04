within ModelicaAerospace.Environment.Gravity;
function inverseSquareGravity "Calculate spherical-Earth gravity magnitude"
  input ModelicaAerospace.Types.Length altitude;
  input ModelicaAerospace.Types.Length referenceRadius =
    ModelicaAerospace.Constants.WGS84.semiMajorAxis;
  output ModelicaAerospace.Types.Acceleration gravity;
algorithm
  assert(referenceRadius + altitude > 0, "Radius from Earth center must be positive");
  gravity := ModelicaAerospace.Constants.WGS84.gravitationalParameter
    / ((referenceRadius + altitude) * (referenceRadius + altitude));
end inverseSquareGravity;
