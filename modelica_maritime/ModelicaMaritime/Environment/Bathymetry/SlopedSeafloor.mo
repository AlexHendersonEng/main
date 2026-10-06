within ModelicaMaritime.Environment.Bathymetry;
block SlopedSeafloor "Planar-sloped seafloor and vehicle altitude"
  parameter ModelicaMaritime.Types.Length referenceDepth = 100;
  parameter Real slopeNorth = 0;
  parameter Real slopeEast = 0;
  ModelicaMaritime.Interfaces.Vector3Input positionNED(each unit="m");
  ModelicaMaritime.Interfaces.RealOutput depth(unit="m");
  ModelicaMaritime.Interfaces.RealOutput altitude(unit="m");
equation
  depth = ModelicaMaritime.Environment.Bathymetry.slopedSeafloorDepth(
    positionNED,
    referenceDepth,
    slopeNorth,
    slopeEast);
  altitude = ModelicaMaritime.Coordinates.altitudeAboveSeafloor(
    positionNED[3],
    depth);
end SlopedSeafloor;
