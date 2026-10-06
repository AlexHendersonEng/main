within ModelicaMaritime.Environment.Current.Blocks;
block LinearCurrent "Current varying linearly with non-negative depth"
  parameter ModelicaMaritime.Types.Velocity surfaceCurrentNED[3] = {0, 0, 0};
  parameter Real gradientNED[3](each unit="1/s") = {0, 0, 0};
  ModelicaMaritime.Interfaces.RealInput depth(unit="m");
  ModelicaMaritime.Interfaces.Vector3Output velocityNED(each unit="m/s");
equation
  assert(depth >= 0, "LinearCurrent requires non-negative depth");
  velocityNED = surfaceCurrentNED + gradientNED * depth;
end LinearCurrent;
