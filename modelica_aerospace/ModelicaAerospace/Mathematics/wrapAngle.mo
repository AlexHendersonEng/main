within ModelicaAerospace.Mathematics;
function wrapAngle "Wrap an angle to [-pi, pi]"
  input ModelicaAerospace.Types.Angle angle;
  output ModelicaAerospace.Types.Angle wrapped;
algorithm
  wrapped := atan2(sin(angle), cos(angle));
end wrapAngle;
