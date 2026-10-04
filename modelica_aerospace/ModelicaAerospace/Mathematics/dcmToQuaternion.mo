within ModelicaAerospace.Mathematics;
function dcmToQuaternion "Convert a proper active rotation matrix to a quaternion"
  input ModelicaAerospace.Types.Matrix3 dcm;
  output ModelicaAerospace.Types.Quaternion quaternion;
protected
  Real trace;
  Real scale;
algorithm
  assert(
    ModelicaAerospace.Mathematics.isRotationMatrix(dcm),
    "DCM must be an orthogonal proper-rotation matrix");
  trace := dcm[1, 1] + dcm[2, 2] + dcm[3, 3];
  if trace > 0 then
    scale := 2 * sqrt(trace + 1);
    quaternion := {
      0.25 * scale,
      (dcm[3, 2] - dcm[2, 3]) / scale,
      (dcm[1, 3] - dcm[3, 1]) / scale,
      (dcm[2, 1] - dcm[1, 2]) / scale};
  elseif dcm[1, 1] > dcm[2, 2] and dcm[1, 1] > dcm[3, 3] then
    scale := 2 * sqrt(1 + dcm[1, 1] - dcm[2, 2] - dcm[3, 3]);
    quaternion := {
      (dcm[3, 2] - dcm[2, 3]) / scale,
      0.25 * scale,
      (dcm[1, 2] + dcm[2, 1]) / scale,
      (dcm[1, 3] + dcm[3, 1]) / scale};
  elseif dcm[2, 2] > dcm[3, 3] then
    scale := 2 * sqrt(1 + dcm[2, 2] - dcm[1, 1] - dcm[3, 3]);
    quaternion := {
      (dcm[1, 3] - dcm[3, 1]) / scale,
      (dcm[1, 2] + dcm[2, 1]) / scale,
      0.25 * scale,
      (dcm[2, 3] + dcm[3, 2]) / scale};
  else
    scale := 2 * sqrt(1 + dcm[3, 3] - dcm[1, 1] - dcm[2, 2]);
    quaternion := {
      (dcm[2, 1] - dcm[1, 2]) / scale,
      (dcm[1, 3] + dcm[3, 1]) / scale,
      (dcm[2, 3] + dcm[3, 2]) / scale,
      0.25 * scale};
  end if;
  quaternion := ModelicaAerospace.Mathematics.normalizeQuaternion(quaternion);
end dcmToQuaternion;
