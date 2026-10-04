within ModelicaAerospace.Coordinates.Blocks;
block ECEFToNEDVector "Rotate a vector from ECEF to local NED"
  ModelicaAerospace.Interfaces.Vector3Input ecef;
  ModelicaAerospace.Interfaces.RealInput latitude(unit="rad");
  ModelicaAerospace.Interfaces.RealInput longitude(unit="rad");
  ModelicaAerospace.Interfaces.Vector3Output ned;
equation
  ned = ModelicaAerospace.Coordinates.ecefToNEDVector(ecef, latitude, longitude);
end ECEFToNEDVector;
