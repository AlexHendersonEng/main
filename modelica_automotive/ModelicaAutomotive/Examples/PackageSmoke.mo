within ModelicaAutomotive.Examples;
model PackageSmoke "Minimal package-loading and signal simulation check"
  parameter Real scale=2 "Constant output value";
  output Real y "Smoke-test output";
equation
  y = scale;
  annotation (
    experiment(StartTime=0, StopTime=0.1, Tolerance=1e-8, Interval=0.1),
    Documentation(info="<html>
<p>Minimal model used to verify that the complete package hierarchy and its
Modelica Standard Library dependency load in supported compilers.</p>
</html>"));
end PackageSmoke;
