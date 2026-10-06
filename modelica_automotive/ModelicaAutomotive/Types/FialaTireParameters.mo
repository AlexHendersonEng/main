within ModelicaAutomotive.Types;
record FialaTireParameters "Fiala brush tire stiffness parameters"
  Real longitudinalStiffness(unit="N") = 100000
    "Longitudinal brush stiffness";
  Real corneringStiffness(unit="N/rad") = 80000
    "Lateral brush stiffness";
end FialaTireParameters;
