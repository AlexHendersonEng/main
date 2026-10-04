within ModelicaAerospace.Environment.Atmosphere.Blocks;
block StandardAtmosphere "U.S. Standard Atmosphere 1976 properties"
  extends ModelicaAerospace.Interfaces.PartialEnvironment;
  ModelicaAerospace.Interfaces.RealOutput dynamicViscosity(unit="Pa.s");
equation
  temperature =
    ModelicaAerospace.Environment.Atmosphere.standardAtmosphereProperty(altitude, 1);
  pressure =
    ModelicaAerospace.Environment.Atmosphere.standardAtmosphereProperty(altitude, 2);
  density =
    ModelicaAerospace.Environment.Atmosphere.standardAtmosphereProperty(altitude, 3);
  speedOfSound =
    ModelicaAerospace.Environment.Atmosphere.standardAtmosphereProperty(altitude, 4);
  dynamicViscosity =
    ModelicaAerospace.Environment.Atmosphere.standardAtmosphereProperty(altitude, 5);
end StandardAtmosphere;
