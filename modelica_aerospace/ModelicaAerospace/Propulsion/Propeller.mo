within ModelicaAerospace.Propulsion;
block Propeller "Power-based propeller thrust abstraction"
  parameter Real maximumShaftPower(unit="W") = 1;
  parameter Real efficiency(min=0, max=1) = 0.8;
  parameter ModelicaAerospace.Types.Velocity minimumAirspeed = 10;
  ModelicaAerospace.Interfaces.RealInput throttle;
  ModelicaAerospace.Interfaces.RealInput airspeed(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput thrust(unit="N");
equation
  assert(maximumShaftPower >= 0, "Maximum shaft power must be non-negative");
  assert(minimumAirspeed > 0, "Minimum propeller airspeed must be positive");
  thrust = efficiency * maximumShaftPower
    * ModelicaAerospace.Mathematics.clamp(throttle, 0, 1)
    / max(abs(airspeed), minimumAirspeed);
end Propeller;
