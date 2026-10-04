within ModelicaAerospace.Coordinates;
function nedToGeodetic "Convert local NED displacement to WGS-84 geodetic position"
  input ModelicaAerospace.Types.Length ned[3];
  input ModelicaAerospace.Types.GeodeticPosition reference;
  output ModelicaAerospace.Types.GeodeticPosition geodetic;
protected
  ModelicaAerospace.Types.Length referenceECEF[3];
  ModelicaAerospace.Types.Length ecef[3];
algorithm
  referenceECEF := ModelicaAerospace.Coordinates.geodeticToECEF(reference);
  ecef := referenceECEF + ModelicaAerospace.Coordinates.nedToECEFVector(
    ned,
    reference.latitude,
    reference.longitude);
  geodetic := ModelicaAerospace.Coordinates.ecefToGeodetic(ecef);
end nedToGeodetic;
