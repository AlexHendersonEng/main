within ModelicaMaritime.Hydrodynamics;
function planarDampingLoad "Dissipative linear and quadratic planar load"
  input ModelicaMaritime.Types.Matrix3 linearDamping;
  input ModelicaMaritime.Types.Matrix3 quadraticDamping;
  input ModelicaMaritime.Types.Vector3 relativeVelocityBody "{u_r, v_r, r}";
  output ModelicaMaritime.Types.Vector3 dampingLoad "{X, Y, N}";
protected
  ModelicaMaritime.Types.Vector3 quadraticVelocity;
algorithm
  quadraticVelocity := {
    abs(relativeVelocityBody[1]) * relativeVelocityBody[1],
    abs(relativeVelocityBody[2]) * relativeVelocityBody[2],
    abs(relativeVelocityBody[3]) * relativeVelocityBody[3]};
  dampingLoad := -linearDamping * relativeVelocityBody
    - quadraticDamping * quadraticVelocity;
end planarDampingLoad;
