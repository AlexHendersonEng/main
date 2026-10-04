within ModelicaAerospace.Environment.Atmosphere;
function geopotentialAltitude "Convert geometric altitude to geopotential altitude"
  input ModelicaAerospace.Types.Length geometricAltitude;
  output ModelicaAerospace.Types.Length altitude;
protected
  constant ModelicaAerospace.Types.Length radius =
    ModelicaAerospace.Constants.StandardAtmosphere.geopotentialRadius;
algorithm
  assert(
    geometricAltitude > -radius,
    "Geometric altitude must be greater than the negative geopotential Earth radius");
  altitude := radius * geometricAltitude / (radius + geometricAltitude);
end geopotentialAltitude;
