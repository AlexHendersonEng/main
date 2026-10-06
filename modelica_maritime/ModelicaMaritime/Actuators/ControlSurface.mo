within ModelicaMaritime.Actuators;
block ControlSurface "Low-order rudder, hydroplane, or control-fin load"
  parameter ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter Real area(unit="m2", min=0) = 1;
  parameter Real liftSlope(unit="1/rad") = 6.283185307179586;
  parameter Real dragCoefficient(min=0) = 0.01;
  parameter ModelicaMaritime.Types.Vector3 normalDirectionBody = {0, 1, 0}
    "Nominal positive lift direction; use body y for rudder or body z for plane";
  parameter ModelicaMaritime.Types.Vector3 applicationPointBody = {0, 0, 0};
  ModelicaMaritime.Interfaces.RealInput deflection(unit="rad");
  ModelicaMaritime.Interfaces.Vector3Input relativeVelocityBody(each unit="m/s");
  ModelicaMaritime.Interfaces.Vector6Output generalizedLoadBody;
  ModelicaMaritime.Interfaces.RealOutput lift(unit="N");
  ModelicaMaritime.Interfaces.RealOutput drag(unit="N");
protected
  Real normalNorm;
  Real speedSquared(unit="m2/s2");
  Real axialSign;
  Real forceBody[3](each unit="N");
  Real momentBody[3](each unit="N.m");
equation
  normalNorm = sqrt(normalDirectionBody * normalDirectionBody);
  assert(density > 0, "Control-surface fluid density must be positive");
  assert(area > 0, "Control-surface area must be positive");
  assert(normalNorm > ModelicaMaritime.Constants.small, "Surface normal must be nonzero");
  speedSquared = relativeVelocityBody[1] * relativeVelocityBody[1];
  axialSign = if relativeVelocityBody[1] >= 0 then 1 else -1;
  lift = 0.5 * density * area * speedSquared * liftSlope * deflection;
  drag = 0.5 * density * area * speedSquared * dragCoefficient;
  forceBody = lift * normalDirectionBody / normalNorm
    + {-axialSign * drag, 0, 0};
  momentBody = cross(applicationPointBody, forceBody);
  generalizedLoadBody = {
    forceBody[1],
    forceBody[2],
    forceBody[3],
    momentBody[1],
    momentBody[2],
    momentBody[3]};
end ControlSurface;
