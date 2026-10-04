within ModelicaAerospace.Propulsion;
block FirstOrderEngine "First-order normalized engine spool"
  parameter Real timeConstant(unit="s") = 1;
  parameter ModelicaAerospace.Types.Force maximumThrust = 1;
  parameter Real maximumFuelFlow(unit="kg/s") = 0;
  parameter Real initialSpool(min=0, max=1) = 0;
  ModelicaAerospace.Interfaces.RealInput throttle;
  input Boolean enabled;
  ModelicaAerospace.Interfaces.RealOutput spool;
  ModelicaAerospace.Interfaces.RealOutput thrust(unit="N");
  ModelicaAerospace.Interfaces.RealOutput fuelFlow(unit="kg/s");
protected
  Real spoolState(start=initialSpool, fixed=true);
  Real demandedSpool;
equation
  assert(timeConstant > 0, "Engine time constant must be positive");
  assert(maximumThrust >= 0, "Maximum thrust must be non-negative");
  assert(maximumFuelFlow >= 0, "Maximum fuel flow must be non-negative");
  demandedSpool = if enabled then
    ModelicaAerospace.Mathematics.clamp(throttle, 0, 1)
    else 0;
  der(spoolState) = (demandedSpool - spoolState) / timeConstant;
  spool = spoolState;
  thrust = maximumThrust * spoolState;
  fuelFlow = maximumFuelFlow * spoolState;
end FirstOrderEngine;
