within ModelicaAerospace.Tests.Environment;
model SeaLevel "Sea-level atmosphere and equatorial gravity"
  extends ModelicaAerospace.Tests.Environment.EnvironmentValidation(
    altitude=0,
    latitude=0);
end SeaLevel;
