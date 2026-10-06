within ModelicaMaritime.Types;
record FossenHydrodynamicProperties "Added inertia and damping for 6-DoF marine craft"
  ModelicaMaritime.Types.Matrix6 addedMass = zeros(6, 6)
    "Symmetric positive-semidefinite added-inertia matrix";
  ModelicaMaritime.Types.Matrix6 linearDamping = zeros(6, 6)
    "Linear damping matrix";
  ModelicaMaritime.Types.Matrix6 quadraticDamping = zeros(6, 6)
    "Quadratic damping applied to abs(nu_r).*nu_r";
end FossenHydrodynamicProperties;
