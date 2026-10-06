within ModelicaMaritime.Coordinates;
function rotationBodyToNED321 "Active body-to-NED rotation for roll, pitch, heading"
  input ModelicaMaritime.Types.Angle euler321[3] "{roll, pitch, heading}";
  output ModelicaMaritime.Types.Matrix3 rotation;
protected
  Real cPhi;
  Real sPhi;
  Real cTheta;
  Real sTheta;
  Real cPsi;
  Real sPsi;
algorithm
  cPhi := cos(euler321[1]);
  sPhi := sin(euler321[1]);
  cTheta := cos(euler321[2]);
  sTheta := sin(euler321[2]);
  cPsi := cos(euler321[3]);
  sPsi := sin(euler321[3]);
  rotation := [
    cTheta * cPsi,
    sPhi * sTheta * cPsi - cPhi * sPsi,
    cPhi * sTheta * cPsi + sPhi * sPsi;
    cTheta * sPsi,
    sPhi * sTheta * sPsi + cPhi * cPsi,
    cPhi * sTheta * sPsi - sPhi * cPsi;
    -sTheta,
    sPhi * cTheta,
    cPhi * cTheta];
end rotationBodyToNED321;
