within ModelicaAerospace.Environment.Wind.Blocks;
block SteadyWind "Constant wind vector in NED coordinates"
  parameter ModelicaAerospace.Types.Velocity windNED[3] = {0, 0, 0};
  ModelicaAerospace.Interfaces.Vector3Output velocityNED(each unit="m/s");
equation
  velocityNED = windNED;
end SteadyWind;
