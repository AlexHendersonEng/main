within ModelicaMaritime.Coordinates;
function nedToBodyVector "Rotate a NED-frame vector into body axes"
  input ModelicaMaritime.Types.Vector3 ned;
  input ModelicaMaritime.Types.Angle euler321[3] "{roll, pitch, heading}";
  output ModelicaMaritime.Types.Vector3 body;
protected
  ModelicaMaritime.Types.Matrix3 rotation;
algorithm
  rotation := ModelicaMaritime.Coordinates.rotationBodyToNED321(euler321);
  body := transpose(rotation) * ned;
end nedToBodyVector;
