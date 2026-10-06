within ModelicaAutomotive.Powertrain;
block EnergyStorage "Integrate usable fuel or electrical energy"
  parameter Real capacity(unit="J") = 1e8;
  parameter Real initialStateOfCharge(min=0, max=1) = 1;
  ModelicaAutomotive.Interfaces.RealInput sourcePower(unit="W")
    "Positive discharge power and negative charging power";
  ModelicaAutomotive.Interfaces.RealOutput energy(unit="J");
  ModelicaAutomotive.Interfaces.RealOutput stateOfCharge;
protected
  Real energyState(start=capacity * initialStateOfCharge, fixed=true, unit="J");
equation
  assert(capacity > 0, "capacity must be positive");
  der(energyState) = -sourcePower;
  energy = energyState;
  stateOfCharge = energyState / capacity;
end EnergyStorage;
