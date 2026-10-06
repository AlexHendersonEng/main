within ModelicaMaritime.Mathematics;
function skewMatrix "Return the cross-product matrix such that skew(v) * w = v cross w"
  input ModelicaMaritime.Types.Vector3 value;
  output ModelicaMaritime.Types.Matrix3 skew;
algorithm
  skew := [0, -value[3], value[2];
           value[3], 0, -value[1];
           -value[2], value[1], 0];
end skewMatrix;
