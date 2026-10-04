within ModelicaAerospace.Mathematics;
function euler321ToQuaternion
  "Convert roll, pitch, yaw using the aerospace 3-2-1 sequence"
  input ModelicaAerospace.Types.Angle roll;
  input ModelicaAerospace.Types.Angle pitch;
  input ModelicaAerospace.Types.Angle yaw;
  output ModelicaAerospace.Types.Quaternion quaternion;
protected
  Real cr;
  Real sr;
  Real cp;
  Real sp;
  Real cy;
  Real sy;
algorithm
  cr := cos(roll / 2);
  sr := sin(roll / 2);
  cp := cos(pitch / 2);
  sp := sin(pitch / 2);
  cy := cos(yaw / 2);
  sy := sin(yaw / 2);
  quaternion := {
    cr * cp * cy + sr * sp * sy,
    sr * cp * cy - cr * sp * sy,
    cr * sp * cy + sr * cp * sy,
    cr * cp * sy - sr * sp * cy};
  quaternion := ModelicaAerospace.Mathematics.normalizeQuaternion(quaternion);
end euler321ToQuaternion;
