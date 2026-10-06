within ModelicaAutomotive.Types;
record MagicFormulaTireParameters "Compact pure-slip Magic Formula parameters"
  Real longitudinalStiffness(unit="N") = 90000
    "Longitudinal small-slip stiffness";
  Real lateralStiffness(unit="N/rad") = 70000
    "Lateral small-slip stiffness";
  Real longitudinalShape(min=0) = 1.65;
  Real lateralShape(min=0) = 1.3;
  Real longitudinalCurvature = 0.2;
  Real lateralCurvature = -1.6;
end MagicFormulaTireParameters;
