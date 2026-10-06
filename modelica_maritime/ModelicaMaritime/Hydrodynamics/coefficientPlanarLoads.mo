within ModelicaMaritime.Hydrodynamics;
function coefficientPlanarLoads "Scale nondimensional planar coefficients into forces and yaw moment"
  input Real coefficients[3] "{CX, CY, CN}";
  input ModelicaMaritime.Types.Density density;
  input ModelicaMaritime.Types.Length referenceLength;
  input ModelicaMaritime.Types.Length referenceDraft;
  input ModelicaMaritime.Types.Velocity relativeSpeed(min=0);
  output ModelicaMaritime.Types.Vector3 generalizedLoad "{X, Y, N}";
protected
  Real dynamicPressure(unit="Pa");
  Real referenceArea(unit="m2");
algorithm
  assert(density > 0, "Fluid density must be positive");
  assert(referenceLength > 0, "Reference length must be positive");
  assert(referenceDraft > 0, "Reference draft must be positive");
  assert(relativeSpeed >= 0, "Relative speed must be non-negative");
  dynamicPressure := 0.5 * density * relativeSpeed * relativeSpeed;
  referenceArea := referenceLength * referenceDraft;
  generalizedLoad := {
    dynamicPressure * referenceArea * coefficients[1],
    dynamicPressure * referenceArea * coefficients[2],
    dynamicPressure * referenceArea * referenceLength * coefficients[3]};
end coefficientPlanarLoads;
