within ModelicaMaritime.Environment.Bathymetry;
block FlatSeafloor "Constant-depth seafloor and vehicle altitude"
  parameter ModelicaMaritime.Types.Length seafloorDepth = 100;
  ModelicaMaritime.Interfaces.Vector3Input positionNED(each unit="m");
  ModelicaMaritime.Interfaces.RealOutput depth(unit="m");
  ModelicaMaritime.Interfaces.RealOutput altitude(unit="m");
equation
  assert(seafloorDepth >= 0, "Seafloor depth must be non-negative");
  depth = seafloorDepth;
  altitude = ModelicaMaritime.Coordinates.altitudeAboveSeafloor(
    positionNED[3],
    depth);
end FlatSeafloor;
