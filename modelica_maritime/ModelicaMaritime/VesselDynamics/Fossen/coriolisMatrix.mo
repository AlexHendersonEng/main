within ModelicaMaritime.VesselDynamics.Fossen;
function coriolisMatrix "Skew-symmetric 6-DoF Coriolis matrix for a constant inertia matrix"
  input ModelicaMaritime.Types.Matrix6 massMatrix;
  input ModelicaMaritime.Types.GeneralizedVelocity velocityBody;
  output ModelicaMaritime.Types.Matrix6 coriolis;
protected
  Real momentum[6];
  ModelicaMaritime.Types.Matrix3 skewLinearMomentum;
  ModelicaMaritime.Types.Matrix3 skewAngularMomentum;
algorithm
  momentum := massMatrix * velocityBody;
  skewLinearMomentum := ModelicaMaritime.Mathematics.skewMatrix(momentum[1:3]);
  skewAngularMomentum := ModelicaMaritime.Mathematics.skewMatrix(momentum[4:6]);
  coriolis := [
    0, 0, 0,
      -skewLinearMomentum[1, 1],
      -skewLinearMomentum[1, 2],
      -skewLinearMomentum[1, 3];
    0, 0, 0,
      -skewLinearMomentum[2, 1],
      -skewLinearMomentum[2, 2],
      -skewLinearMomentum[2, 3];
    0, 0, 0,
      -skewLinearMomentum[3, 1],
      -skewLinearMomentum[3, 2],
      -skewLinearMomentum[3, 3];
    -skewLinearMomentum[1, 1],
      -skewLinearMomentum[1, 2],
      -skewLinearMomentum[1, 3],
      -skewAngularMomentum[1, 1],
      -skewAngularMomentum[1, 2],
      -skewAngularMomentum[1, 3];
    -skewLinearMomentum[2, 1],
      -skewLinearMomentum[2, 2],
      -skewLinearMomentum[2, 3],
      -skewAngularMomentum[2, 1],
      -skewAngularMomentum[2, 2],
      -skewAngularMomentum[2, 3];
    -skewLinearMomentum[3, 1],
      -skewLinearMomentum[3, 2],
      -skewLinearMomentum[3, 3],
      -skewAngularMomentum[3, 1],
      -skewAngularMomentum[3, 2],
      -skewAngularMomentum[3, 3]];
end coriolisMatrix;
