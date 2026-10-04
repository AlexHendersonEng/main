within ModelicaAerospace.Tests.Coordinates;
model GenericPosition "Generic WGS-84 position"
  extends ModelicaAerospace.Tests.Coordinates.CoordinateValidation(
    latitude=0.7,
    longitude=-2.4,
    altitude=1234);
end GenericPosition;
