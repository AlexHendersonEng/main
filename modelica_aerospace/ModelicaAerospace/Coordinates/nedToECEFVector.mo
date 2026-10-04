within ModelicaAerospace.Coordinates;
function nedToECEFVector "Rotate a local NED vector into ECEF components"
  input ModelicaAerospace.Types.Vector3 ned;
  input ModelicaAerospace.Types.Angle latitude;
  input ModelicaAerospace.Types.Angle longitude;
  output ModelicaAerospace.Types.Vector3 ecef;
protected
  ModelicaAerospace.Types.Matrix3 dcm;
algorithm
  dcm := ModelicaAerospace.Coordinates.nedToECEFMatrix(latitude, longitude);
  for row in 1:3 loop
    ecef[row] := dcm[row, 1] * ned[1] + dcm[row, 2] * ned[2] + dcm[row, 3] * ned[3];
  end for;
end nedToECEFVector;
