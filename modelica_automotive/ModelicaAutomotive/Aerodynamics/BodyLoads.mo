within ModelicaAutomotive.Aerodynamics;
block BodyLoads "Aerodynamic body forces and moments from relative air velocity"
  parameter ModelicaAutomotive.Types.AerodynamicParameters parameters;
  parameter ModelicaAutomotive.Types.Velocity velocityRegularization = 0.1;
  ModelicaAutomotive.Interfaces.Vector3Input relativeAirVelocityBody(each unit="m/s")
    "Vehicle velocity relative to air, expressed in body axes";
  ModelicaAutomotive.Interfaces.RealInput airDensity(unit="kg/m3");
  ModelicaAutomotive.Interfaces.Vector3Output forceBody(each unit="N");
  ModelicaAutomotive.Interfaces.Vector3Output momentBody(each unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput dynamicPressure(unit="Pa");
  ModelicaAutomotive.Interfaces.RealOutput sideslip(unit="rad");
protected
  Real speedSquared(unit="m2/s2");
  Real referenceSpeed(unit="m/s");
  Real coefficientForce[3];
  Real coefficientMoment[3];
  Real applicationMoment[3];
equation
  assert(parameters.referenceArea >= 0, "referenceArea must not be negative");
  assert(parameters.referenceLength > 0, "referenceLength must be positive");
  assert(parameters.dragCoefficient >= 0, "dragCoefficient must not be negative");
  assert(airDensity >= 0, "airDensity must not be negative");
  assert(velocityRegularization > 0, "velocityRegularization must be positive");
  speedSquared =
    relativeAirVelocityBody[1] * relativeAirVelocityBody[1]
    + relativeAirVelocityBody[2] * relativeAirVelocityBody[2]
    + relativeAirVelocityBody[3] * relativeAirVelocityBody[3];
  referenceSpeed = sqrt(
    relativeAirVelocityBody[1] * relativeAirVelocityBody[1]
    + velocityRegularization * velocityRegularization);
  dynamicPressure = 0.5 * airDensity * speedSquared;
  sideslip = atan2(relativeAirVelocityBody[2], referenceSpeed);
  coefficientForce = {
    -parameters.dragCoefficient
      * ModelicaAutomotive.Mathematics.regularizedSign(
        relativeAirVelocityBody[1],
        velocityRegularization),
    -parameters.sideForceDerivative * sideslip,
    parameters.liftCoefficient};
  forceBody = dynamicPressure * parameters.referenceArea * coefficientForce;
  coefficientMoment = dynamicPressure * parameters.referenceArea
    * parameters.referenceLength * parameters.momentCoefficients;
  applicationMoment = ModelicaAutomotive.Mathematics.cross3(
    parameters.applicationPointBody,
    forceBody);
  momentBody = coefficientMoment + applicationMoment;
end BodyLoads;
