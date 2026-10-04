within ModelicaAerospace.Tests.Common;
block EnvironmentStub "Concrete environment interface"
  extends ModelicaAerospace.Interfaces.PartialEnvironment;
equation
  temperature = 288.15 - 0.0065 * altitude;
  pressure = 101325;
  density = 1.225;
  speedOfSound = 340.294;
end EnvironmentStub;
