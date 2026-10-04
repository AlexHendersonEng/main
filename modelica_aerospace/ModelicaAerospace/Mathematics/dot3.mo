within ModelicaAerospace.Mathematics;
function dot3 "Dot product of two three-vectors"
  input ModelicaAerospace.Types.Vector3 a;
  input ModelicaAerospace.Types.Vector3 b;
  output Real result;
algorithm
  result := a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
end dot3;
