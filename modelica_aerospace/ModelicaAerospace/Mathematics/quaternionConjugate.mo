within ModelicaAerospace.Mathematics;
function quaternionConjugate "Conjugate a scalar-first quaternion"
  input ModelicaAerospace.Types.Quaternion quaternion;
  output ModelicaAerospace.Types.Quaternion conjugate;
algorithm
  conjugate := {quaternion[1], -quaternion[2], -quaternion[3], -quaternion[4]};
end quaternionConjugate;
