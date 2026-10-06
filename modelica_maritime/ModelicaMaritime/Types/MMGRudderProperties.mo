within ModelicaMaritime.Types;
record MMGRudderProperties "Low-order MMG rudder inflow and interaction properties"
  Real area(unit="m2") = 1 "Rudder area";
  ModelicaMaritime.Types.Length longitudinalPosition = -1
    "Rudder x position from body reference; aft is negative";
  ModelicaMaritime.Types.Length hullForcePosition = 0
    "Effective hull lateral-force x position";
  Real liftGradient(min=0) = 6.13 "Rudder normal-force slope";
  Real axialInflowFactor(min=0) = 1 "Propeller axial velocity to rudder inflow";
  Real lateralInflowFactor(min=0) = 1 "Hull lateral velocity to rudder inflow";
  Real hullForceIncrease(min=0) = 0 "Hull-rudder lateral interaction aH";
  Real steeringResistanceDeduction(min=0, max=1) = 0
    "Rudder surge-force deduction tR";
end MMGRudderProperties;
