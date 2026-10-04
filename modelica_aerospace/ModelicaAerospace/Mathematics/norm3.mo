within ModelicaAerospace.Mathematics;
function norm3 "Euclidean norm of a three-vector"
  input ModelicaAerospace.Types.Vector3 value;
  output Real magnitude;
algorithm
  magnitude := sqrt(ModelicaAerospace.Mathematics.dot3(value, value));
end norm3;
