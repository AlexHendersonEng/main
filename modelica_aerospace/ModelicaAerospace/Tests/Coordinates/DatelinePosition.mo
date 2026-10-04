within ModelicaAerospace.Tests.Coordinates;
model DatelinePosition "Near-dateline WGS-84 position"
  extends ModelicaAerospace.Tests.Coordinates.CoordinateValidation(
    latitude=0.2,
    longitude=3.141592652589793,
    altitude=35000);
end DatelinePosition;
