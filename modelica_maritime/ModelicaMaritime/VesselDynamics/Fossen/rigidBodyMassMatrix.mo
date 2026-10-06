within ModelicaMaritime.VesselDynamics.Fossen;
function rigidBodyMassMatrix "Rigid-body 6-DoF inertia matrix about the body reference point"
  input ModelicaMaritime.Types.MassProperties massProperties;
  output ModelicaMaritime.Types.Matrix6 massMatrix;
protected
  ModelicaMaritime.Types.Matrix3 skewCenter;
  ModelicaMaritime.Types.Matrix3 inertiaReference;
algorithm
  assert(massProperties.mass > 0, "Rigid-body mass must be positive");
  skewCenter := ModelicaMaritime.Mathematics.skewMatrix(
    massProperties.centerOfGravityBody);
  inertiaReference := massProperties.inertiaBody
    - massProperties.mass * skewCenter * skewCenter;
  massMatrix := [
    massProperties.mass, 0, 0,
      -massProperties.mass * skewCenter[1, 1],
      -massProperties.mass * skewCenter[1, 2],
      -massProperties.mass * skewCenter[1, 3];
    0, massProperties.mass, 0,
      -massProperties.mass * skewCenter[2, 1],
      -massProperties.mass * skewCenter[2, 2],
      -massProperties.mass * skewCenter[2, 3];
    0, 0, massProperties.mass,
      -massProperties.mass * skewCenter[3, 1],
      -massProperties.mass * skewCenter[3, 2],
      -massProperties.mass * skewCenter[3, 3];
    massProperties.mass * skewCenter[1, 1],
      massProperties.mass * skewCenter[1, 2],
      massProperties.mass * skewCenter[1, 3],
      inertiaReference[1, 1], inertiaReference[1, 2], inertiaReference[1, 3];
    massProperties.mass * skewCenter[2, 1],
      massProperties.mass * skewCenter[2, 2],
      massProperties.mass * skewCenter[2, 3],
      inertiaReference[2, 1], inertiaReference[2, 2], inertiaReference[2, 3];
    massProperties.mass * skewCenter[3, 1],
      massProperties.mass * skewCenter[3, 2],
      massProperties.mass * skewCenter[3, 3],
      inertiaReference[3, 1], inertiaReference[3, 2], inertiaReference[3, 3]];
end rigidBodyMassMatrix;
