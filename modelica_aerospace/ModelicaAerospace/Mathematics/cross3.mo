within ModelicaAerospace.Mathematics;
function cross3 "Cross product of two three-vectors"
  input ModelicaAerospace.Types.Vector3 a;
  input ModelicaAerospace.Types.Vector3 b;
  output ModelicaAerospace.Types.Vector3 result;
algorithm
  result[1] := a[2] * b[3] - a[3] * b[2];
  result[2] := a[3] * b[1] - a[1] * b[3];
  result[3] := a[1] * b[2] - a[2] * b[1];
end cross3;
