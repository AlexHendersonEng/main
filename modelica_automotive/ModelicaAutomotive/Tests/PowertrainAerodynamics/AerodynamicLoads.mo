within ModelicaAutomotive.Tests.PowertrainAerodynamics;
model AerodynamicLoads "Evaluate force coefficients and application-point moments"
  ModelicaAutomotive.Aerodynamics.BodyLoads loads(
    parameters(
      referenceArea=2,
      referenceLength=2.5,
      dragCoefficient=0.3,
      sideForceDerivative=0.8,
      liftCoefficient=-0.1,
      momentCoefficients={0.01, -0.02, 0.03},
      applicationPointBody={1, 0, 0.2}));
  output Real dynamicPressure;
  output Real sideslip;
  output Real forceBody[3];
  output Real momentBody[3];
  output Real sideslipIntegral(start=0, fixed=true);
equation
  loads.relativeAirVelocityBody = {30, 2, 0};
  loads.airDensity = 1.2;
  dynamicPressure = loads.dynamicPressure;
  sideslip = loads.sideslip;
  forceBody = loads.forceBody;
  momentBody = loads.momentBody;
  der(sideslipIntegral) = loads.sideslip;
end AerodynamicLoads;
