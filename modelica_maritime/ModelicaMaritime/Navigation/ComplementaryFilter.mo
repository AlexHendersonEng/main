within ModelicaMaritime.Navigation;
block ComplementaryFilter "Fuse propagated rate with an absolute scalar measurement"
  parameter Real bandwidth(unit="1/s") = 1;
  parameter Real initialEstimate = 0;
  parameter Boolean angularState = false;
  ModelicaMaritime.Interfaces.RealInput rate;
  ModelicaMaritime.Interfaces.RealInput absoluteMeasurement;
  ModelicaMaritime.Interfaces.RealOutput estimate;
protected
  Real estimateState(start=initialEstimate, fixed=true);
  Real correctionError;
equation
  assert(bandwidth > 0, "Complementary-filter bandwidth must be positive");
  correctionError = if angularState then
    atan2(
      sin(absoluteMeasurement - estimateState),
      cos(absoluteMeasurement - estimateState))
    else absoluteMeasurement - estimateState;
  der(estimateState) = rate + bandwidth * correctionError;
  estimate = if angularState then
    ModelicaMaritime.Mathematics.wrapHeading(estimateState) else estimateState;
end ComplementaryFilter;
