within ModelicaMaritime.Propulsion;
block EnergyStore "Bounded energy state with charge and discharge power limits"
  parameter Real capacity(unit="J") = 1;
  parameter Real initialEnergy(unit="J") = capacity;
  parameter Real maximumDischargePower(unit="W", min=0) = capacity;
  parameter Real maximumChargePower(unit="W", min=0) = capacity;
  ModelicaMaritime.Interfaces.RealInput requestedPower(unit="W")
    "Positive discharges the store; negative charges it";
  input Boolean enabled;
  ModelicaMaritime.Interfaces.RealOutput deliveredPower(unit="W");
  ModelicaMaritime.Interfaces.RealOutput energy(unit="J");
  ModelicaMaritime.Interfaces.RealOutput stateOfCharge;
  output Boolean empty;
  output Boolean full;
protected
  Real energyState(start=initialEnergy, fixed=true, unit="J");
  Real limitedPower(unit="W");
equation
  assert(capacity > 0, "Energy capacity must be positive");
  assert(initialEnergy >= 0 and initialEnergy <= capacity, "Initial energy is invalid");
  limitedPower = min(max(requestedPower, -maximumChargePower), maximumDischargePower);
  deliveredPower = if not enabled then 0
    else if energyState <= 0 and limitedPower > 0 then 0
    else if energyState >= capacity and limitedPower < 0 then 0
    else limitedPower;
  der(energyState) = -deliveredPower;
  energy = min(max(energyState, 0), capacity);
  stateOfCharge = energy / capacity;
  empty = energyState <= 0;
  full = energyState >= capacity;
end EnergyStore;
