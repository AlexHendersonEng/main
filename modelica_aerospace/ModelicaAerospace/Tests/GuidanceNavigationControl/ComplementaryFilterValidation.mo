within ModelicaAerospace.Tests.GuidanceNavigationControl;
model ComplementaryFilterValidation "Exercise scalar and wrapped-angle fusion"
  ModelicaAerospace.Navigation.ComplementaryFilter scalarFilter(
    bandwidth=2,
    initialEstimate=0);
  ModelicaAerospace.Navigation.ComplementaryFilter angleFilter(
    bandwidth=2,
    initialEstimate=3.1,
    angularState=true);
  output Real scalarEstimate;
  output Real angleEstimate;
equation
  scalarFilter.rate = 0;
  scalarFilter.absoluteMeasurement = 10;
  scalarEstimate = scalarFilter.estimate;
  angleFilter.rate = 0;
  angleFilter.absoluteMeasurement = -3.1;
  angleEstimate = angleFilter.estimate;
end ComplementaryFilterValidation;
