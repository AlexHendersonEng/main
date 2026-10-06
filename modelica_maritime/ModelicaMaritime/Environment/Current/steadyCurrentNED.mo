within ModelicaMaritime.Environment.Current;
function steadyCurrentNED "Build a NED current vector from horizontal speed and direction"
  input ModelicaMaritime.Types.Velocity horizontalSpeed(min=0);
  input ModelicaMaritime.Types.Angle direction
    "Direction toward which current flows, clockwise from north";
  input ModelicaMaritime.Types.Velocity verticalSpeed = 0 "Positive down";
  output ModelicaMaritime.Types.Velocity currentNED[3];
algorithm
  assert(horizontalSpeed >= 0, "Horizontal current speed must be non-negative");
  currentNED := {
    horizontalSpeed * cos(direction),
    horizontalSpeed * sin(direction),
    verticalSpeed};
end steadyCurrentNED;
