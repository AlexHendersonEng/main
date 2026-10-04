within ModelicaAerospace.Mathematics;
function normalizeQuaternion "Normalize a scalar-first quaternion"
  input ModelicaAerospace.Types.Quaternion quaternion;
  output ModelicaAerospace.Types.Quaternion normalized;
protected
  Real magnitude;
algorithm
  magnitude := sqrt(
    quaternion[1] * quaternion[1]
    + quaternion[2] * quaternion[2]
    + quaternion[3] * quaternion[3]
    + quaternion[4] * quaternion[4]);
  assert(
    magnitude > ModelicaAerospace.Constants.Numerics.small,
    "Cannot normalize a near-zero quaternion");
  normalized := quaternion / magnitude;
end normalizeQuaternion;
