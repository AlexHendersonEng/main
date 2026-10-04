within ModelicaAerospace.Mathematics;
function quaternionToEuler321
  "Convert a quaternion to roll, pitch, yaw using the 3-2-1 sequence"
  input ModelicaAerospace.Types.Quaternion quaternion;
  output ModelicaAerospace.Types.Angle euler[3] "{roll, pitch, yaw}";
protected
  ModelicaAerospace.Types.Matrix3 dcm;
algorithm
  dcm := ModelicaAerospace.Mathematics.quaternionToDCM(quaternion);
  euler[1] := atan2(dcm[3, 2], dcm[3, 3]);
  euler[2] := asin(ModelicaAerospace.Mathematics.clamp(-dcm[3, 1], -1, 1));
  euler[3] := atan2(dcm[2, 1], dcm[1, 1]);
end quaternionToEuler321;
