within ModelicaAerospace.Coordinates;
function eciToECEFVector "Rotate an ECI vector into ECEF components"
  input ModelicaAerospace.Types.Vector3 eci;
  input Real elapsedTime(unit="s");
  input ModelicaAerospace.Types.Angle angleAtEpoch = 0;
  output ModelicaAerospace.Types.Vector3 ecef;
protected
  ModelicaAerospace.Types.Matrix3 dcm;
algorithm
  dcm := ModelicaAerospace.Coordinates.eciToECEFMatrix(elapsedTime, angleAtEpoch);
  for row in 1:3 loop
    ecef[row] := dcm[row, 1] * eci[1] + dcm[row, 2] * eci[2] + dcm[row, 3] * eci[3];
  end for;
end eciToECEFVector;
