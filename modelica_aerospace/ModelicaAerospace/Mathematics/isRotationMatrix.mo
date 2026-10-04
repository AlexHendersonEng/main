within ModelicaAerospace.Mathematics;
function isRotationMatrix "Check orthogonality and positive unit determinant"
  input ModelicaAerospace.Types.Matrix3 matrix;
  input Real tolerance = ModelicaAerospace.Constants.Numerics.rotationOrthogonalityTolerance;
  output Boolean valid;
protected
  Real product;
  Real target;
  Real largestError;
  Real determinant;
algorithm
  largestError := 0;
  for row in 1:3 loop
    for column in 1:3 loop
      product := 0;
      for index in 1:3 loop
        product := product + matrix[row, index] * matrix[column, index];
      end for;
      target := if row == column then 1 else 0;
      if abs(product - target) > largestError then
        largestError := abs(product - target);
      end if;
    end for;
  end for;
  determinant := ModelicaAerospace.Mathematics.determinant3(matrix);
  valid := largestError <= tolerance and abs(determinant - 1) <= tolerance;
end isRotationMatrix;
