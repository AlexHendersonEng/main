within ModelicaAerospace.Coordinates;
function geodeticToNED "Convert geodetic position to local NED displacement"
  input ModelicaAerospace.Types.GeodeticPosition geodetic;
  input ModelicaAerospace.Types.GeodeticPosition reference;
  output ModelicaAerospace.Types.Length ned[3];
protected
  ModelicaAerospace.Types.Length ecef[3];
  ModelicaAerospace.Types.Length referenceECEF[3];
algorithm
  ecef := ModelicaAerospace.Coordinates.geodeticToECEF(geodetic);
  referenceECEF := ModelicaAerospace.Coordinates.geodeticToECEF(reference);
  ned := ModelicaAerospace.Coordinates.ecefToNEDVector(
    ecef - referenceECEF,
    reference.latitude,
    reference.longitude);
end geodeticToNED;
