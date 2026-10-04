within ModelicaAerospace.Coordinates;
function ecefToNEDVector "Rotate an ECEF vector into local NED components"
  input ModelicaAerospace.Types.Vector3 ecef;
  input ModelicaAerospace.Types.Angle latitude;
  input ModelicaAerospace.Types.Angle longitude;
  output ModelicaAerospace.Types.Vector3 ned;
protected
  ModelicaAerospace.Types.Matrix3 dcm;
algorithm
  dcm := ModelicaAerospace.Coordinates.ecefToNEDMatrix(latitude, longitude);
  for row in 1:3 loop
    ned[row] := dcm[row, 1] * ecef[1] + dcm[row, 2] * ecef[2] + dcm[row, 3] * ecef[3];
  end for;
end ecefToNEDVector;
