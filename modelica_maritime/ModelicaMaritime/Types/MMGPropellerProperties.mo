within ModelicaMaritime.Types;
record MMGPropellerProperties "Low-order MMG propeller and interaction properties"
  ModelicaMaritime.Types.Length diameter = 1 "Propeller diameter";
  Real wakeFraction(min=0, max=1) = 0 "Nominal wake fraction";
  Real thrustDeduction(min=0, max=1) = 0 "Hull thrust-deduction fraction";
  Real thrustCoefficient[3] = {0.2, 0, 0}
    "Open-water KT polynomial {k0, k1, k2} in advance ratio";
end MMGPropellerProperties;
