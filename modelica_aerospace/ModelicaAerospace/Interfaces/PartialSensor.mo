within ModelicaAerospace.Interfaces;
partial block PartialSensor "Common three-axis sensor interface"
  ModelicaAerospace.Interfaces.Vector3Input truth "Noise-free physical quantity";
  ModelicaAerospace.Interfaces.Vector3Output measurement "Reported measurement";
end PartialSensor;
