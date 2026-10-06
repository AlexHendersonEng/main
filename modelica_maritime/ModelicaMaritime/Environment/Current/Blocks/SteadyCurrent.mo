within ModelicaMaritime.Environment.Current.Blocks;
block SteadyCurrent "Constant current vector in NED coordinates"
  parameter ModelicaMaritime.Types.Velocity currentNED[3] = {0, 0, 0};
  ModelicaMaritime.Interfaces.Vector3Output velocityNED(each unit="m/s");
equation
  velocityNED = currentNED;
end SteadyCurrent;
