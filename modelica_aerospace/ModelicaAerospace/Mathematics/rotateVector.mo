within ModelicaAerospace.Mathematics;
function rotateVector "Apply an active quaternion rotation to a three-vector"
  input ModelicaAerospace.Types.Quaternion quaternion;
  input ModelicaAerospace.Types.Vector3 vector;
  output ModelicaAerospace.Types.Vector3 rotated;
protected
  ModelicaAerospace.Types.Matrix3 dcm;
algorithm
  dcm := ModelicaAerospace.Mathematics.quaternionToDCM(quaternion);
  for row in 1:3 loop
    rotated[row] :=
      dcm[row, 1] * vector[1]
      + dcm[row, 2] * vector[2]
      + dcm[row, 3] * vector[3];
  end for;
end rotateVector;
