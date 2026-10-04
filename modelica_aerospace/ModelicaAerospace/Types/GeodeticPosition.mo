within ModelicaAerospace.Types;
record GeodeticPosition "WGS-84 geodetic position"
  ModelicaAerospace.Types.Angle latitude(min=-1.570796326794897, max=1.570796326794897)
    "Geodetic latitude; north positive";
  ModelicaAerospace.Types.Angle longitude(min=-3.141592653589793, max=3.141592653589793)
    "Longitude; east positive";
  ModelicaAerospace.Types.Length altitude "Ellipsoidal altitude above WGS-84";
end GeodeticPosition;
