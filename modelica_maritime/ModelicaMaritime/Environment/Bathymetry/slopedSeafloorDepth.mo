within ModelicaMaritime.Environment.Bathymetry;
function slopedSeafloorDepth "Evaluate planar seafloor depth at north/east position"
  input ModelicaMaritime.Types.Vector3 positionNED;
  input ModelicaMaritime.Types.Length referenceDepth;
  input Real slopeNorth = 0 "Depth change per north distance";
  input Real slopeEast = 0 "Depth change per east distance";
  output ModelicaMaritime.Types.Length seafloorDepth;
algorithm
  seafloorDepth := referenceDepth
    + slopeNorth * positionNED[1]
    + slopeEast * positionNED[2];
  assert(seafloorDepth >= 0, "Bathymetry produced seafloor above the water surface");
end slopedSeafloorDepth;
