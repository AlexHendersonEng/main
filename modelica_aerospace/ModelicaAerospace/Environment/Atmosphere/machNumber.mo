within ModelicaAerospace.Environment.Atmosphere;
function machNumber "Calculate Mach number from true airspeed and local speed of sound"
  input ModelicaAerospace.Types.Velocity trueAirspeed;
  input ModelicaAerospace.Types.Velocity speedOfSound;
  output Real mach;
algorithm
  assert(speedOfSound > 0, "Speed of sound must be positive");
  mach := abs(trueAirspeed) / speedOfSound;
end machNumber;
