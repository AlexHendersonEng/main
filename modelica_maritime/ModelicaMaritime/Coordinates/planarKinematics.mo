within ModelicaMaritime.Coordinates;
function planarKinematics "Map planar body velocity {u, v, r} to {northRate, eastRate, headingRate}"
  input ModelicaMaritime.Types.Angle heading;
  input Real velocityBody[3] "Planar body velocity {u, v, r}";
  output Real poseRateNED[3] "Planar state rate {north, east, heading}";
algorithm
  poseRateNED := {
    cos(heading) * velocityBody[1] - sin(heading) * velocityBody[2],
    sin(heading) * velocityBody[1] + cos(heading) * velocityBody[2],
    velocityBody[3]};
end planarKinematics;
