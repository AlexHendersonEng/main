within ModelicaAerospace.Coordinates.Blocks;
block NEDToECEFVector "Rotate a vector from local NED to ECEF"
  ModelicaAerospace.Interfaces.Vector3Input ned;
  ModelicaAerospace.Interfaces.RealInput latitude(unit="rad");
  ModelicaAerospace.Interfaces.RealInput longitude(unit="rad");
  ModelicaAerospace.Interfaces.Vector3Output ecef;
equation
  ecef = ModelicaAerospace.Coordinates.nedToECEFVector(ned, latitude, longitude);
end NEDToECEFVector;
