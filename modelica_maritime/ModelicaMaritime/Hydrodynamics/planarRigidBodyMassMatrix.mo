within ModelicaMaritime.Hydrodynamics;
function planarRigidBodyMassMatrix "Rigid-body surge, sway, and yaw inertia matrix"
  input ModelicaMaritime.Types.PlanarMassProperties massProperties;
  output ModelicaMaritime.Types.Matrix3 massMatrix;
algorithm
  assert(massProperties.mass > 0, "Planar vehicle mass must be positive");
  assert(massProperties.yawInertia > 0, "Planar vehicle yaw inertia must be positive");
  massMatrix := [
    massProperties.mass, 0, 0;
    0, massProperties.mass,
      massProperties.mass * massProperties.centerOfGravityX;
    0, massProperties.mass * massProperties.centerOfGravityX,
      massProperties.yawInertia
      + massProperties.mass * massProperties.centerOfGravityX
        * massProperties.centerOfGravityX];
end planarRigidBodyMassMatrix;
