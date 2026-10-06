within ModelicaAutomotive.Mathematics;
function normalizeQuaternion "Normalize a scalar-first quaternion"
  input ModelicaAutomotive.Types.Quaternion quaternion;
  output ModelicaAutomotive.Types.Quaternion normalized;
protected
  Real magnitude;
algorithm
  magnitude := sqrt(
    quaternion[1] * quaternion[1]
    + quaternion[2] * quaternion[2]
    + quaternion[3] * quaternion[3]
    + quaternion[4] * quaternion[4]);
  assert(
    magnitude > ModelicaAutomotive.Constants.small,
    "Cannot normalize a near-zero quaternion");
  normalized := quaternion / magnitude;
end normalizeQuaternion;
