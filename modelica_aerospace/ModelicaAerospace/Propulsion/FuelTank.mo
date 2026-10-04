within ModelicaAerospace.Propulsion;
block FuelTank "Integrate fuel consumption without allowing negative mass"
  parameter ModelicaAerospace.Types.Mass initialFuelMass = 1;
  ModelicaAerospace.Interfaces.RealInput requestedFuelFlow(unit="kg/s");
  ModelicaAerospace.Interfaces.RealOutput fuelMass(unit="kg");
  ModelicaAerospace.Interfaces.RealOutput deliveredFuelFlow(unit="kg/s");
  ModelicaAerospace.Interfaces.RealOutput consumedFuelMass(unit="kg");
  output Boolean empty;
protected
  Real massState(start=initialFuelMass, fixed=true);
equation
  assert(initialFuelMass >= 0, "Initial fuel mass must be non-negative");
  deliveredFuelFlow = if massState > 0 then max(requestedFuelFlow, 0) else 0;
  der(massState) = -deliveredFuelFlow;
  fuelMass = max(massState, 0);
  consumedFuelMass = initialFuelMass - fuelMass;
  empty = massState <= 0;
end FuelTank;
