within ModelicaMaritime.Environment.Wind.Blocks;
block SteadyWind "Constant wind vector in NED coordinates"
  parameter ModelicaMaritime.Types.Velocity windNED[3] = {0, 0, 0};
  ModelicaMaritime.Interfaces.Vector3Output velocityNED(each unit="m/s");
equation
  velocityNED = windNED;
end SteadyWind;
