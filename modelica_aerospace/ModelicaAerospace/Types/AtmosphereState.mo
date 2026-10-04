within ModelicaAerospace.Types;
record AtmosphereState "Local atmospheric properties"
  ModelicaAerospace.Types.Temperature temperature = 288.15;
  ModelicaAerospace.Types.Pressure pressure = 101325;
  ModelicaAerospace.Types.Density density = 1.225;
  ModelicaAerospace.Types.Velocity speedOfSound = 340.294;
  ModelicaAerospace.Types.DynamicViscosity dynamicViscosity = 1.7894e-5;
end AtmosphereState;
