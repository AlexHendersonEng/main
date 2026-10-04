within ModelicaAerospace.Environment.Wind.Blocks;
block DrydenTurbulence "Seeded low-altitude Dryden turbulence shaping filters"
  parameter Integer seed = 1;
  parameter ModelicaAerospace.Types.Velocity windSpeedAt20Feet = 15.24;
  ModelicaAerospace.Interfaces.RealInput altitude(unit="m");
  ModelicaAerospace.Interfaces.RealInput trueAirspeed(unit="m/s");
  ModelicaAerospace.Interfaces.Vector3Output turbulenceBody(each unit="m/s");
protected
  ModelicaAerospace.Types.Length lengthScale[3];
  ModelicaAerospace.Types.Velocity sigma[3];
  Real rate[3](each unit="1/s");
  Real forcing[3];
  Real firstState[3](each start=0, each fixed=true);
  Real secondState[2](each start=0, each fixed=true);
equation
  assert(trueAirspeed > 0, "Dryden true airspeed must be positive");
  (lengthScale, sigma) = ModelicaAerospace.Environment.Wind.drydenParameters(
    altitude,
    windSpeedAt20Feet);
  for index in 1:3 loop
    rate[index] = trueAirspeed / lengthScale[index];
    forcing[index] =
      ModelicaAerospace.Environment.Wind.deterministicNoise(time, seed, index);
    der(firstState[index]) = rate[index] * (forcing[index] - firstState[index]);
  end for;
  der(secondState[1]) = rate[2] * (firstState[2] - secondState[1]);
  der(secondState[2]) = rate[3] * (firstState[3] - secondState[2]);
  turbulenceBody[1] = sigma[1] * firstState[1];
  turbulenceBody[2] = sigma[2]
    * (sqrt(3) * firstState[2] + (1 - sqrt(3)) * secondState[1]);
  turbulenceBody[3] = sigma[3]
    * (sqrt(3) * firstState[3] + (1 - sqrt(3)) * secondState[2]);
end DrydenTurbulence;
