within ModelicaAutomotive.Mathematics;
function quaternionToDCM "Convert a scalar-first quaternion to an active rotation matrix"
  input ModelicaAutomotive.Types.Quaternion quaternion;
  output ModelicaAutomotive.Types.Matrix3 dcm;
protected
  ModelicaAutomotive.Types.Quaternion q;
algorithm
  q := ModelicaAutomotive.Mathematics.normalizeQuaternion(quaternion);
  dcm := [
    1 - 2 * (q[3] * q[3] + q[4] * q[4]),
    2 * (q[2] * q[3] - q[1] * q[4]),
    2 * (q[2] * q[4] + q[1] * q[3]);
    2 * (q[2] * q[3] + q[1] * q[4]),
    1 - 2 * (q[2] * q[2] + q[4] * q[4]),
    2 * (q[3] * q[4] - q[1] * q[2]);
    2 * (q[2] * q[4] - q[1] * q[3]),
    2 * (q[3] * q[4] + q[1] * q[2]),
    1 - 2 * (q[2] * q[2] + q[3] * q[3])];
end quaternionToDCM;
