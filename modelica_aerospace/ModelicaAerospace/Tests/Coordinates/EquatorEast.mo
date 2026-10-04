within ModelicaAerospace.Tests.Coordinates;
model EquatorEast "Equator at 90 degrees east"
  extends ModelicaAerospace.Tests.Coordinates.CoordinateValidation(
    latitude=0,
    longitude=1.570796326794897,
    altitude=0);
end EquatorEast;
