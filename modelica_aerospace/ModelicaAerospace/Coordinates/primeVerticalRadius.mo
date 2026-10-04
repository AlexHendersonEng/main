within ModelicaAerospace.Coordinates;
function primeVerticalRadius "WGS-84 prime-vertical radius of curvature"
  input ModelicaAerospace.Types.Angle latitude;
  output ModelicaAerospace.Types.Length radius;
algorithm
  radius := ModelicaAerospace.Constants.WGS84.semiMajorAxis / sqrt(
    1
    - ModelicaAerospace.Constants.WGS84.firstEccentricitySquared
    * sin(latitude) * sin(latitude));
end primeVerticalRadius;
