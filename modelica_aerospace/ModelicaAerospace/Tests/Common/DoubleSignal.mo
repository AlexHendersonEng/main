within ModelicaAerospace.Tests.Common;
block DoubleSignal "Concrete scalar transform used to validate common connectors"
  extends ModelicaAerospace.Interfaces.PartialScalarTransform;
equation
  y = 2 * u;
end DoubleSignal;
