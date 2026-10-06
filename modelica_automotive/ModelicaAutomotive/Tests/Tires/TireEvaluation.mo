within ModelicaAutomotive.Tests.Tires;
model TireEvaluation "Parameter-driven evaluation of tire kinematics and force models"
  parameter Real longitudinalVelocity=20;
  parameter Real lateralVelocity=-1;
  parameter Real angularVelocity=70;
  parameter Real rollingRadius=0.3;
  parameter Real slipRatio=0.08;
  parameter Real slipAngle=0.06;
  parameter Real normalLoad=4000;
  parameter Real frictionCoefficient=0.9;
  ModelicaAutomotive.Tires.Kinematics kinematics(
    rollingRadius=rollingRadius,
    velocityRegularization=0.1);
  ModelicaAutomotive.Tires.LinearTire linear;
  ModelicaAutomotive.Tires.FialaTire fiala;
  ModelicaAutomotive.Tires.MagicFormulaTire magicFormula;
  output Real calculatedSlipRatio;
  output Real calculatedSlipAngle;
  output Real integratedSlipAngle(start=0, fixed=true);
  output Real linearForce[2];
  output Real fialaForce[2];
  output Real magicFormulaForce[2];
  output Real utilization[3];
equation
  kinematics.longitudinalVelocity = longitudinalVelocity;
  kinematics.lateralVelocity = lateralVelocity;
  kinematics.angularVelocity = angularVelocity;
  calculatedSlipRatio = kinematics.slipRatio;
  calculatedSlipAngle = kinematics.slipAngle;
  der(integratedSlipAngle) = calculatedSlipAngle;
  linear.slipRatio = slipRatio;
  linear.slipAngle = slipAngle;
  linear.normalLoad = normalLoad;
  linear.frictionCoefficient = frictionCoefficient;
  fiala.slipRatio = slipRatio;
  fiala.slipAngle = slipAngle;
  fiala.normalLoad = normalLoad;
  fiala.frictionCoefficient = frictionCoefficient;
  magicFormula.slipRatio = slipRatio;
  magicFormula.slipAngle = slipAngle;
  magicFormula.normalLoad = normalLoad;
  magicFormula.frictionCoefficient = frictionCoefficient;
  linearForce = {linear.longitudinalForce, linear.lateralForce};
  fialaForce = {fiala.longitudinalForce, fiala.lateralForce};
  magicFormulaForce = {
    magicFormula.longitudinalForce,
    magicFormula.lateralForce};
  utilization = {
    linear.utilization,
    fiala.utilization,
    magicFormula.utilization};
end TireEvaluation;
