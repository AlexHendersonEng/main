within ModelicaAutomotive.Tests.Tires;
model TireLoadValidation "Validate friction-limited tire force load scaling"
  ModelicaAutomotive.Tires.FialaTire lowLoad;
  ModelicaAutomotive.Tires.FialaTire highLoad;
  output Real lowLoadImpulse(start=0, fixed=true);
  output Real highLoadImpulse(start=0, fixed=true);
equation
  lowLoad.slipRatio = 0.5;
  lowLoad.slipAngle = 0;
  lowLoad.normalLoad = 2000;
  lowLoad.frictionCoefficient = 0.8;
  highLoad.slipRatio = 0.5;
  highLoad.slipAngle = 0;
  highLoad.normalLoad = 4000;
  highLoad.frictionCoefficient = 0.8;
  der(lowLoadImpulse) = lowLoad.longitudinalForce;
  der(highLoadImpulse) = highLoad.longitudinalForce;
end TireLoadValidation;
