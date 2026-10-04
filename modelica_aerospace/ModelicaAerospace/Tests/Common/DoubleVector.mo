within ModelicaAerospace.Tests.Common;
block DoubleVector "Concrete vector transform used to validate array connectors"
  extends ModelicaAerospace.Interfaces.PartialVectorTransform;
equation
  y = 2 * u;
end DoubleVector;
