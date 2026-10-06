within ModelicaAutomotive.Tests.WheelsBrakes;
model BrakeEvaluation "Evaluate ideal, dynamic, and friction-limited brakes"
  ModelicaAutomotive.Brakes.IdealBrake ideal(
    maximumTorque=3000,
    speedRegularization=0.1);
  ModelicaAutomotive.Brakes.FirstOrderBrake dynamic(
    maximumTorque=3000,
    timeConstant=0.5,
    speedRegularization=0.1);
  ModelicaAutomotive.Brakes.FrictionLimitedBrake frictionLimited(
    maximumCommandTorque=5000,
    liningFrictionCoefficient=0.4,
    effectiveRadius=0.15,
    speedRegularization=0.1);
  output Real idealTorque;
  output Real dynamicTorque;
  output Real application;
  output Real limitedTorque;
  output Real limitedMagnitude;
equation
  ideal.command = 0.6;
  ideal.angularVelocity = 20;
  dynamic.command = 1;
  dynamic.angularVelocity = 20;
  frictionLimited.command = 1;
  frictionLimited.clampForce = 20000;
  frictionLimited.angularVelocity = 20;
  idealTorque = ideal.brakeTorque;
  dynamicTorque = dynamic.brakeTorque;
  application = dynamic.application;
  limitedTorque = frictionLimited.brakeTorque;
  limitedMagnitude = frictionLimited.appliedMagnitude;
end BrakeEvaluation;
