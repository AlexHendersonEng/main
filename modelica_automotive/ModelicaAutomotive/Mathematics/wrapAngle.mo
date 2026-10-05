within ModelicaAutomotive.Mathematics;
function wrapAngle "Wrap an angle to the interval [-pi, pi]"
  input ModelicaAutomotive.Types.Angle angle;
  output ModelicaAutomotive.Types.Angle wrapped;
algorithm
  wrapped := atan2(sin(angle), cos(angle));
end wrapAngle;
