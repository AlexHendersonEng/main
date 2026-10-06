within ModelicaMaritime.Hydrodynamics;
function planarCoriolisMatrix
  "Skew-symmetric planar Coriolis matrix from inertia and velocity"
  input ModelicaMaritime.Types.Matrix3 massMatrix;
  input ModelicaMaritime.Types.Vector3 velocityBody "{u, v, r}";
  output ModelicaMaritime.Types.Matrix3 coriolis;
protected
  ModelicaMaritime.Types.Vector3 momentum;
algorithm
  momentum := massMatrix * velocityBody;
  coriolis := [
    0, 0, -momentum[2];
    0, 0, momentum[1];
    momentum[2], -momentum[1], 0];
end planarCoriolisMatrix;
