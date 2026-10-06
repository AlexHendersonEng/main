within ModelicaMaritime.Environment.Current.Blocks;
block TabulatedCurrent "Depth-profile current with linear interpolation and endpoint clamping"
  parameter Integer nDepths(min=2) = 2;
  parameter ModelicaMaritime.Types.Length depthGrid[nDepths] = {0, 100};
  parameter ModelicaMaritime.Types.Velocity currentTable[nDepths, 3] =
    [0, 0, 0; 0, 0, 0];
  ModelicaMaritime.Interfaces.RealInput depth(unit="m");
  ModelicaMaritime.Interfaces.Vector3Output velocityNED(each unit="m/s");
equation
  velocityNED = ModelicaMaritime.Environment.Current.tabulatedCurrentProfile(
    depth,
    depthGrid,
    currentTable);
end TabulatedCurrent;
