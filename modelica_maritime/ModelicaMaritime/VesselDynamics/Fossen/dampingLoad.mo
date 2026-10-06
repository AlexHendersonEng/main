within ModelicaMaritime.VesselDynamics.Fossen;
function dampingLoad "Dissipative linear and quadratic 6-DoF load"
  input ModelicaMaritime.Types.Matrix6 linearDamping;
  input ModelicaMaritime.Types.Matrix6 quadraticDamping;
  input ModelicaMaritime.Types.GeneralizedVelocity relativeVelocityBody;
  output ModelicaMaritime.Types.GeneralizedForce loadBody;
protected
  Real quadraticVelocity[6];
algorithm
  for index in 1:6 loop
    quadraticVelocity[index] :=
      abs(relativeVelocityBody[index]) * relativeVelocityBody[index];
  end for;
  loadBody := -linearDamping * relativeVelocityBody
    - quadraticDamping * quadraticVelocity;
end dampingLoad;
