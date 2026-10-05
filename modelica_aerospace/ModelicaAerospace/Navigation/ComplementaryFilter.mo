within ModelicaAerospace.Navigation;
block ComplementaryFilter "Fuse a propagated rate with an absolute measurement"
  parameter Real bandwidth(unit="1/s") = 1;
  parameter Real initialEstimate = 0;
  parameter Boolean angularState = false;
  ModelicaAerospace.Interfaces.RealInput rate;
  ModelicaAerospace.Interfaces.RealInput absoluteMeasurement;
  ModelicaAerospace.Interfaces.RealOutput estimate;
protected
  Real estimateState(start=initialEstimate, fixed=true);
  Real correctionError;
equation
  assert(bandwidth > 0, "Complementary-filter bandwidth must be positive");
  correctionError = if angularState then
    ModelicaAerospace.Mathematics.wrapAngle(absoluteMeasurement - estimateState)
    else absoluteMeasurement - estimateState;
  der(estimateState) = rate + bandwidth * correctionError;
  estimate = if angularState then
    ModelicaAerospace.Mathematics.wrapAngle(estimateState)
    else estimateState;
end ComplementaryFilter;
