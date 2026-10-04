within ModelicaAerospace.Mathematics;
function normalizeVector3 "Return a unit three-vector"
  input ModelicaAerospace.Types.Vector3 value;
  output ModelicaAerospace.Types.Vector3 normalized;
protected
  Real magnitude;
algorithm
  magnitude := ModelicaAerospace.Mathematics.norm3(value);
  assert(
    magnitude > ModelicaAerospace.Constants.Numerics.small,
    "Cannot normalize a near-zero vector");
  normalized := value / magnitude;
end normalizeVector3;
