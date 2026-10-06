within ModelicaMaritime.Coordinates;
function bodyToNEDVector "Rotate a body-frame vector into NED"
  input ModelicaMaritime.Types.Vector3 body;
  input ModelicaMaritime.Types.Angle euler321[3] "{roll, pitch, heading}";
  output ModelicaMaritime.Types.Vector3 ned;
protected
  ModelicaMaritime.Types.Matrix3 rotation;
algorithm
  rotation := ModelicaMaritime.Coordinates.rotationBodyToNED321(euler321);
  ned := rotation * body;
end bodyToNEDVector;
