within ModelicaMaritime.Coordinates;
function rigidBodyKinematics321 "Map body velocity to NED position and Euler 3-2-1 rates"
  input ModelicaMaritime.Types.Angle euler321[3] "{roll, pitch, heading}";
  input ModelicaMaritime.Types.GeneralizedVelocity velocityBody
    "{u, v, w, p, q, r}";
  output Real stateRate[6]
    "{northRate, eastRate, downRate, rollRate, pitchRate, headingRate}";
protected
  ModelicaMaritime.Types.Vector3 translationalRate;
  ModelicaMaritime.Types.AngularVelocity angularRate[3];
algorithm
  translationalRate := ModelicaMaritime.Coordinates.bodyToNEDVector(
    {velocityBody[1], velocityBody[2], velocityBody[3]},
    euler321);
  angularRate := ModelicaMaritime.Coordinates.bodyRatesToEuler321Rates(
    euler321,
    {velocityBody[4], velocityBody[5], velocityBody[6]});
  stateRate := {
    translationalRate[1],
    translationalRate[2],
    translationalRate[3],
    angularRate[1],
    angularRate[2],
    angularRate[3]};
end rigidBodyKinematics321;
