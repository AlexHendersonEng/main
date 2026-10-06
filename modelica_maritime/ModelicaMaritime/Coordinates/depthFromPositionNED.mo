within ModelicaMaritime.Coordinates;
function depthFromPositionNED "Return signed depth from NED position; positive below the surface"
  input ModelicaMaritime.Types.Length positionNED[3];
  output ModelicaMaritime.Types.Length depth;
algorithm
  depth := positionNED[3];
end depthFromPositionNED;
