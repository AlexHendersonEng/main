within ModelicaMaritime.Tests.Coordinates;
model EulerSingularity "Exercise the explicit Euler-rate singularity guard"
  output Real eulerRate[3];
equation
  eulerRate = ModelicaMaritime.Coordinates.bodyRatesToEuler321Rates(
    {0, 1.570796326794897, 0},
    {0, 0, 1});
end EulerSingularity;
