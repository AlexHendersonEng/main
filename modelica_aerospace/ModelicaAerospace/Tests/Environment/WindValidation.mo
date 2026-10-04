within ModelicaAerospace.Tests.Environment;
model WindValidation "Exercise steady wind, shear, gust, and Dryden turbulence"
  ModelicaAerospace.Environment.Wind.Blocks.SteadyWind steady(
    windNED={12, -3, 0});
  ModelicaAerospace.Environment.Wind.Blocks.LinearWindShear shear(
    referenceWindNED={10, 0, 0},
    referenceAltitude=100,
    gradientNED={0.01, -0.005, 0});
  ModelicaAerospace.Environment.Wind.Blocks.OneMinusCosineGust gust(
    startTime=2,
    duration=4,
    amplitudeNED={6, 0, -2});
  ModelicaAerospace.Environment.Wind.Blocks.DrydenTurbulence dryden(
    seed=17,
    windSpeedAt20Feet=15.24);
  ModelicaAerospace.Environment.Wind.Blocks.DrydenTurbulence differentSeed(
    seed=29,
    windSpeedAt20Feet=15.24);
  output Real steadyWind[3];
  output Real shearWind[3];
  output Real gustWind[3];
  output Real forcing[3];
  output Real turbulence[3];
  output Real otherTurbulence[3];
  output Real drydenLengthScale[3];
  output Real drydenSigma[3];
equation
  shear.altitude = 350;
  dryden.altitude = 100;
  dryden.trueAirspeed = 70;
  differentSeed.altitude = 100;
  differentSeed.trueAirspeed = 70;
  steadyWind = steady.velocityNED;
  shearWind = shear.velocityNED;
  gustWind = gust.gustNED;
  for index in 1:3 loop
    forcing[index] =
      ModelicaAerospace.Environment.Wind.deterministicNoise(time, 17, index);
  end for;
  turbulence = dryden.turbulenceBody;
  otherTurbulence = differentSeed.turbulenceBody;
  (drydenLengthScale, drydenSigma) =
    ModelicaAerospace.Environment.Wind.drydenParameters(100, 15.24);
end WindValidation;
