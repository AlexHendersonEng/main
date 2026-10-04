within ModelicaAerospace.Environment.Atmosphere.Blocks;
block AirData "Calculate Mach number and dynamic pressure"
  ModelicaAerospace.Interfaces.RealInput trueAirspeed(unit="m/s");
  ModelicaAerospace.Interfaces.RealInput density(unit="kg/m3");
  ModelicaAerospace.Interfaces.RealInput speedOfSound(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput mach;
  ModelicaAerospace.Interfaces.RealOutput dynamicPressure(unit="Pa");
equation
  mach = ModelicaAerospace.Environment.Atmosphere.machNumber(
    trueAirspeed,
    speedOfSound);
  dynamicPressure =
    ModelicaAerospace.Environment.Atmosphere.dynamicPressure(density, trueAirspeed);
end AirData;
