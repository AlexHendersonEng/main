within ModelicaMaritime.Environment.Wind;
function windVelocityNED "Resolve wind speed and direction into NED velocity"
  input ModelicaMaritime.Types.Velocity speed;
  input ModelicaMaritime.Types.Angle direction
    "Direction toward which wind moves, clockwise from north";
  input ModelicaMaritime.Types.Velocity verticalVelocity = 0
    "Downward velocity is positive";
  output ModelicaMaritime.Types.Velocity velocityNED[3];
algorithm
  assert(speed >= 0, "Wind speed must be non-negative");
  velocityNED := {
    speed * cos(direction),
    speed * sin(direction),
    verticalVelocity};
end windVelocityNED;
