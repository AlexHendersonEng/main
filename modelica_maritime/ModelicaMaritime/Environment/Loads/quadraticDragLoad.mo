within ModelicaMaritime.Environment.Loads;
function quadraticDragLoad "Quadratic drag load opposing translational relative velocity"
  input ModelicaMaritime.Types.Velocity relativeVelocityBody[3]
    "Vehicle velocity minus medium velocity, expressed in body axes";
  input ModelicaMaritime.Types.Density density;
  input Real dragCoefficient[3];
  input Real projectedArea[3](each unit="m2");
  input ModelicaMaritime.Types.Vector3 applicationPointBody;
  output ModelicaMaritime.Types.GeneralizedForce generalizedLoadBody;
protected
  ModelicaMaritime.Types.Force forceBody[3];
  ModelicaMaritime.Types.Torque momentBody[3];
algorithm
  assert(density > 0, "Medium density must be positive");
  for axis in 1:3 loop
    assert(dragCoefficient[axis] >= 0, "Drag coefficients must be non-negative");
    assert(projectedArea[axis] >= 0, "Projected areas must be non-negative");
    forceBody[axis] := -0.5 * density * dragCoefficient[axis]
      * projectedArea[axis] * abs(relativeVelocityBody[axis])
      * relativeVelocityBody[axis];
  end for;
  momentBody := cross(applicationPointBody, forceBody);
  generalizedLoadBody := {
    forceBody[1],
    forceBody[2],
    forceBody[3],
    momentBody[1],
    momentBody[2],
    momentBody[3]};
end quadraticDragLoad;
