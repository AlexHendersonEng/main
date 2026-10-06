within ModelicaMaritime.Mathematics;
function wrapHeading "Wrap a heading to [0, 2*pi)"
  input ModelicaMaritime.Types.Angle heading;
  output ModelicaMaritime.Types.Angle wrapped;
protected
  ModelicaMaritime.Types.Angle signedAngle;
algorithm
  signedAngle := atan2(sin(heading), cos(heading));
  wrapped := if signedAngle < 0 then signedAngle + 6.283185307179586 else signedAngle;
end wrapHeading;
