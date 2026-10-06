within ModelicaMaritime.Tests.Coordinates;
model CoordinateValidation "Validate rotations, kinematics, depth, and altitude conventions"
  parameter Real roll = 0.2;
  parameter Real pitch = -0.3;
  parameter Real heading = 0.7;
  output Real rotation[3, 3];
  output Real identityVector[3];
  output Real yawVector[3];
  output Real roundTrip[3];
  output Real planarRate[3];
  output Real eulerRate[3];
  output Real rigidStateRate[6];
  output Real nearSingularRate[3];
  output Real skew[3, 3];
  output Real wrappedHeading;
  output Real depth;
  output Real altitude;
equation
  rotation = ModelicaMaritime.Coordinates.rotationBodyToNED321(
    {roll, pitch, heading});
  identityVector = ModelicaMaritime.Coordinates.bodyToNEDVector(
    {1, 2, 3},
    {0, 0, 0});
  yawVector = ModelicaMaritime.Coordinates.bodyToNEDVector(
    {1, 0, 0},
    {0, 0, 1.570796326794897});
  roundTrip = ModelicaMaritime.Coordinates.nedToBodyVector(
    ModelicaMaritime.Coordinates.bodyToNEDVector(
      {2, -1, 0.5},
      {roll, pitch, heading}),
    {roll, pitch, heading});
  planarRate = ModelicaMaritime.Coordinates.planarKinematics(
    1.570796326794897,
    {2, 1, 0.3});
  eulerRate = ModelicaMaritime.Coordinates.bodyRatesToEuler321Rates(
    {roll, pitch, heading},
    {0.1, 0.2, 0.3});
  rigidStateRate = ModelicaMaritime.Coordinates.rigidBodyKinematics321(
    {roll, pitch, heading},
    {2, -1, 0.5, 0.1, 0.2, 0.3});
  nearSingularRate = ModelicaMaritime.Coordinates.bodyRatesToEuler321Rates(
    {0.1, 1.560796326794897, 0},
    {0.2, -0.1, 0.05});
  skew = ModelicaMaritime.Mathematics.skewMatrix({1, 2, 3});
  wrappedHeading = ModelicaMaritime.Mathematics.wrapHeading(
    -1.570796326794897);
  depth = ModelicaMaritime.Coordinates.depthFromPositionNED({10, 20, 30});
  altitude = ModelicaMaritime.Coordinates.altitudeAboveSeafloor(30, 100);
end CoordinateValidation;
