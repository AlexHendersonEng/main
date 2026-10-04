within ModelicaAerospace.Propulsion;
block JetThrust "Simple density- and Mach-lapsed jet thrust"
  parameter ModelicaAerospace.Types.Force seaLevelStaticThrust = 1;
  parameter Real densityExponent = 0.7;
  parameter Real machLapse = 0.25;
  ModelicaAerospace.Interfaces.RealInput throttle;
  ModelicaAerospace.Interfaces.RealInput density(unit="kg/m3");
  ModelicaAerospace.Interfaces.RealInput mach;
  ModelicaAerospace.Interfaces.RealOutput thrust(unit="N");
equation
  assert(seaLevelStaticThrust >= 0, "Sea-level static thrust must be non-negative");
  assert(density >= 0, "Density must be non-negative");
  thrust = seaLevelStaticThrust
    * ModelicaAerospace.Mathematics.clamp(throttle, 0, 1)
    * (density / 1.225) ^ densityExponent
    * max(0, 1 - machLapse * max(mach, 0));
end JetThrust;
