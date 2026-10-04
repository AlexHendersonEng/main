within ModelicaAerospace.Interfaces;
partial block PartialEnvironment "Common environment-model interface"
  ModelicaAerospace.Interfaces.RealInput altitude(unit="m")
    "Altitude above the model reference surface";
  ModelicaAerospace.Interfaces.RealOutput temperature(unit="K");
  ModelicaAerospace.Interfaces.RealOutput pressure(unit="Pa");
  ModelicaAerospace.Interfaces.RealOutput density(unit="kg/m3");
  ModelicaAerospace.Interfaces.RealOutput speedOfSound(unit="m/s");
end PartialEnvironment;
