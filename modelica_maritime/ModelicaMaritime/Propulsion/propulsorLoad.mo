within ModelicaMaritime.Propulsion;
function propulsorLoad "Map thrust at a body point and direction to generalized load"
  input ModelicaMaritime.Types.Force thrust;
  input ModelicaMaritime.Types.Vector3 directionBody;
  input ModelicaMaritime.Types.Vector3 applicationPointBody;
  output ModelicaMaritime.Types.GeneralizedForce generalizedLoadBody;
protected
  Real directionNorm;
  ModelicaMaritime.Types.Force forceBody[3];
  ModelicaMaritime.Types.Torque momentBody[3];
algorithm
  directionNorm := sqrt(directionBody * directionBody);
  assert(directionNorm > ModelicaMaritime.Constants.small, "Propulsor direction must be nonzero");
  forceBody := thrust * directionBody / directionNorm;
  momentBody := cross(applicationPointBody, forceBody);
  generalizedLoadBody := {
    forceBody[1],
    forceBody[2],
    forceBody[3],
    momentBody[1],
    momentBody[2],
    momentBody[3]};
end propulsorLoad;
