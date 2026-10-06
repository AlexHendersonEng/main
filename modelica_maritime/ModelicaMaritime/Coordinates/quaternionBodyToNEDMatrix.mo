within ModelicaMaritime.Coordinates;
function quaternionBodyToNEDMatrix
  "Convert scalar-first active body-to-NED quaternion"
  input ModelicaMaritime.Types.Quaternion quaternion;
  output ModelicaMaritime.Types.Matrix3 rotation;
protected
  Real normalized[4];
  Real quaternionNorm;
algorithm
  quaternionNorm := sqrt(
    quaternion[1] * quaternion[1]
    + quaternion[2] * quaternion[2]
    + quaternion[3] * quaternion[3]
    + quaternion[4] * quaternion[4]);
  assert(quaternionNorm > 0, "Quaternion norm must be positive");
  normalized := quaternion / quaternionNorm;
  rotation := [
    1 - 2 * (normalized[3] * normalized[3] + normalized[4] * normalized[4]),
    2 * (normalized[2] * normalized[3] - normalized[1] * normalized[4]),
    2 * (normalized[2] * normalized[4] + normalized[1] * normalized[3]);
    2 * (normalized[2] * normalized[3] + normalized[1] * normalized[4]),
    1 - 2 * (normalized[2] * normalized[2] + normalized[4] * normalized[4]),
    2 * (normalized[3] * normalized[4] - normalized[1] * normalized[2]);
    2 * (normalized[2] * normalized[4] - normalized[1] * normalized[3]),
    2 * (normalized[3] * normalized[4] + normalized[1] * normalized[2]),
    1 - 2 * (normalized[2] * normalized[2] + normalized[3] * normalized[3])];
end quaternionBodyToNEDMatrix;
