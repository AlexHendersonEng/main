within ModelicaAerospace.Coordinates;
function ecefToECIVector "Rotate an ECEF vector into ECI components"
  input ModelicaAerospace.Types.Vector3 ecef;
  input Real elapsedTime(unit="s");
  input ModelicaAerospace.Types.Angle angleAtEpoch = 0;
  output ModelicaAerospace.Types.Vector3 eci;
protected
  ModelicaAerospace.Types.Matrix3 dcm;
algorithm
  dcm := ModelicaAerospace.Coordinates.ecefToECIMatrix(elapsedTime, angleAtEpoch);
  for row in 1:3 loop
    eci[row] := dcm[row, 1] * ecef[1] + dcm[row, 2] * ecef[2] + dcm[row, 3] * ecef[3];
  end for;
end ecefToECIVector;
