within ModelicaMaritime.Coordinates;
function bodyRatesToEuler321Rates "Map body rates {p, q, r} to 3-2-1 Euler angle rates"
  input ModelicaMaritime.Types.Angle euler321[3] "{roll, pitch, heading}";
  input ModelicaMaritime.Types.AngularVelocity angularVelocityBody[3] "{p, q, r}";
  output ModelicaMaritime.Types.AngularVelocity euler321Rate[3];
protected
  Real cPhi;
  Real sPhi;
  Real cTheta;
  Real tTheta;
algorithm
  cPhi := cos(euler321[1]);
  sPhi := sin(euler321[1]);
  cTheta := cos(euler321[2]);
  assert(
    abs(cTheta) > 1e-8,
    "3-2-1 Euler-rate transformation is singular at pitch = +/- pi/2");
  tTheta := sin(euler321[2]) / cTheta;
  euler321Rate := {
    angularVelocityBody[1]
      + sPhi * tTheta * angularVelocityBody[2]
      + cPhi * tTheta * angularVelocityBody[3],
    cPhi * angularVelocityBody[2] - sPhi * angularVelocityBody[3],
    sPhi / cTheta * angularVelocityBody[2]
      + cPhi / cTheta * angularVelocityBody[3]};
end bodyRatesToEuler321Rates;
