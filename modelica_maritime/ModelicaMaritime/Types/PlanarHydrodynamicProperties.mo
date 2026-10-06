within ModelicaMaritime.Types;
record PlanarHydrodynamicProperties
  "Added inertia and damping for surge, sway, and yaw"
  Real addedMass[3, 3] = zeros(3, 3)
    "Symmetric positive-semidefinite added-inertia matrix";
  Real linearDamping[3, 3] = zeros(3, 3)
    "Linear damping matrix mapping {u, v, r} to {X, Y, N}";
  Real quadraticDamping[3, 3] = zeros(3, 3)
    "Quadratic damping matrix applied to abs(nu).*nu";
end PlanarHydrodynamicProperties;
