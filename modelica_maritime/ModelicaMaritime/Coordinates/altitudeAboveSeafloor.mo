within ModelicaMaritime.Coordinates;
function altitudeAboveSeafloor "Return vertical altitude above the seafloor"
  input ModelicaMaritime.Types.Length vehicleDepth "Vehicle depth, positive down";
  input ModelicaMaritime.Types.Length seafloorDepth "Seafloor depth, positive down";
  output ModelicaMaritime.Types.Length altitude;
algorithm
  altitude := seafloorDepth - vehicleDepth;
end altitudeAboveSeafloor;
