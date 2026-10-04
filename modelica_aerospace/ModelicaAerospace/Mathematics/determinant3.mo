within ModelicaAerospace.Mathematics;
function determinant3 "Determinant of a three-by-three matrix"
  input ModelicaAerospace.Types.Matrix3 matrix;
  output Real determinant;
algorithm
  determinant :=
    matrix[1, 1] * (matrix[2, 2] * matrix[3, 3] - matrix[2, 3] * matrix[3, 2])
    - matrix[1, 2] * (matrix[2, 1] * matrix[3, 3] - matrix[2, 3] * matrix[3, 1])
    + matrix[1, 3] * (matrix[2, 1] * matrix[3, 2] - matrix[2, 2] * matrix[3, 1]);
end determinant3;
