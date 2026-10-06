within ModelicaAutomotive.Types;
record LinearTireParameters "Linear tire stiffness parameters"
  Real longitudinalStiffness(unit="N") = 80000
    "Longitudinal force per unit slip ratio";
  Real corneringStiffness(unit="N/rad") = 60000
    "Lateral force per radian slip angle";
end LinearTireParameters;
