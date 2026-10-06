within ModelicaMaritime.Tests.Environment;
model WaveValidation "Validate spectral densities and deterministic wave kinematics"
  ModelicaMaritime.Environment.Waves.RegularWave regular(
    amplitude=1.2,
    period=8,
    direction=0.4,
    phase=0.3);
  ModelicaMaritime.Environment.Waves.DeterministicIrregularWave irregular(
    spectrumKind=3,
    nComponents=4,
    frequencies={0.08, 0.12, 0.16, 0.20},
    frequencyWidth=0.04,
    directions={0, 0.2, -0.1, 0.3},
    phases={0.1, 1.2, 2.1, 3.0},
    significantWaveHeight=3,
    peakPeriod=9);
  ModelicaMaritime.Environment.Waves.DeterministicIrregularWave seeded(
    spectrumKind=3,
    nComponents=3,
    frequencies={0.08, 0.12, 0.16},
    frequencyWidth=0.04,
    directions={0, 0, 0},
    phaseSeed=7,
    significantWaveHeight=3,
    peakPeriod=9);
  output Real piersonMoskowitz;
  output Real bretschneider;
  output Real calmBretschneider;
  output Real jonswap;
  output Real regularElevation;
  output Real regularVelocity[3];
  output Real irregularElevation;
  output Real irregularVelocity[3];
  output Real spectrum[4];
  output Real momentZero;
  output Real realizedHeight;
  output Real seededElevation;
equation
  regular.positionNED = {10, -5, 4};
  irregular.positionNED = {10, -5, 4};
  seeded.positionNED = {0, 0, 0};
  piersonMoskowitz =
    ModelicaMaritime.Environment.Waves.piersonMoskowitzSpectrum(0.12, 15);
  bretschneider =
    ModelicaMaritime.Environment.Waves.bretschneiderSpectrum(0.12, 3, 9);
  calmBretschneider =
    ModelicaMaritime.Environment.Waves.bretschneiderSpectrum(0.12, 0, 9);
  jonswap =
    ModelicaMaritime.Environment.Waves.jonswapSpectrum(0.12, 3, 9, 3.3);
  regularElevation = regular.elevation;
  regularVelocity = regular.waterVelocityNED;
  irregularElevation = irregular.elevation;
  irregularVelocity = irregular.waterVelocityNED;
  spectrum = irregular.spectralDensity;
  momentZero = irregular.spectralMomentZero;
  realizedHeight = irregular.realizedSignificantWaveHeight;
  seededElevation = seeded.elevation;
end WaveValidation;
