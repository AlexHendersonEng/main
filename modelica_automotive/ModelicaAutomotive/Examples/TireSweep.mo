within ModelicaAutomotive.Examples;
model TireSweep "Sweep combined longitudinal and lateral slip"
  ModelicaAutomotive.Tires.LinearTire linear;
  ModelicaAutomotive.Tires.FialaTire fiala;
  ModelicaAutomotive.Tires.MagicFormulaTire magicFormula;
  output Real slipRatio;
  output Real slipAngle(unit="rad");
  output Real linearForce[2];
  output Real fialaForce[2];
  output Real magicFormulaForce[2];
equation
  slipRatio = -0.2 + 0.4 * time / 4;
  slipAngle = -0.15 + 0.3 * time / 4;
  linear.slipRatio = slipRatio;
  linear.slipAngle = slipAngle;
  linear.normalLoad = 4000;
  linear.frictionCoefficient = 0.9;
  fiala.slipRatio = slipRatio;
  fiala.slipAngle = slipAngle;
  fiala.normalLoad = 4000;
  fiala.frictionCoefficient = 0.9;
  magicFormula.slipRatio = slipRatio;
  magicFormula.slipAngle = slipAngle;
  magicFormula.normalLoad = 4000;
  magicFormula.frictionCoefficient = 0.9;
  linearForce = {linear.longitudinalForce, linear.lateralForce};
  fialaForce = {fiala.longitudinalForce, fiala.lateralForce};
  magicFormulaForce = {
    magicFormula.longitudinalForce,
    magicFormula.lateralForce};
  annotation (
    experiment(StartTime=0, StopTime=4, Tolerance=1e-8, Interval=0.01),
    Documentation(info="<html>
<p>Sweeps simultaneous longitudinal and lateral slip through the linear,
Fiala, and compact Magic Formula tire models.</p>
</html>"));
end TireSweep;
