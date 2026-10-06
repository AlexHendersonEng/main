within ModelicaAutomotive.Mathematics;
function solveLinear3 "Solve a nonsingular three-by-three linear system"
  input Real matrix[3, 3];
  input Real rightHandSide[3];
  output Real solution[3];
protected
  Real determinant;
algorithm
  determinant :=
    matrix[1, 1] * (matrix[2, 2] * matrix[3, 3] - matrix[2, 3] * matrix[3, 2])
    - matrix[1, 2] * (matrix[2, 1] * matrix[3, 3] - matrix[2, 3] * matrix[3, 1])
    + matrix[1, 3] * (matrix[2, 1] * matrix[3, 2] - matrix[2, 2] * matrix[3, 1]);
  assert(
    abs(determinant) > ModelicaAutomotive.Constants.small,
    "Linear system matrix must be nonsingular");
  solution[1] := (
    rightHandSide[1] * (matrix[2, 2] * matrix[3, 3] - matrix[2, 3] * matrix[3, 2])
    - matrix[1, 2] * (rightHandSide[2] * matrix[3, 3]
      - matrix[2, 3] * rightHandSide[3])
    + matrix[1, 3] * (rightHandSide[2] * matrix[3, 2]
      - matrix[2, 2] * rightHandSide[3])) / determinant;
  solution[2] := (
    matrix[1, 1] * (rightHandSide[2] * matrix[3, 3]
      - matrix[2, 3] * rightHandSide[3])
    - rightHandSide[1] * (matrix[2, 1] * matrix[3, 3]
      - matrix[2, 3] * matrix[3, 1])
    + matrix[1, 3] * (matrix[2, 1] * rightHandSide[3]
      - rightHandSide[2] * matrix[3, 1])) / determinant;
  solution[3] := (
    matrix[1, 1] * (matrix[2, 2] * rightHandSide[3]
      - rightHandSide[2] * matrix[3, 2])
    - matrix[1, 2] * (matrix[2, 1] * rightHandSide[3]
      - rightHandSide[2] * matrix[3, 1])
    + rightHandSide[1] * (matrix[2, 1] * matrix[3, 2]
      - matrix[2, 2] * matrix[3, 1])) / determinant;
end solveLinear3;
