within ModelicaMaritime.Environment.Waves;
block DeterministicIrregularWave
  "Finite deterministic wave realization from a documented spectrum"
  parameter Integer spectrumKind(min=1, max=3) = 2
    "1 Pierson-Moskowitz, 2 JONSWAP, 3 Bretschneider";
  parameter Integer nComponents(min=1) = 8;
  parameter Real frequencies[nComponents](each unit="Hz", each min=ModelicaMaritime.Constants.small) =
    {0.05 + 0.025 * (index - 1) for index in 1:nComponents};
  parameter Real frequencyWidth(unit="Hz", min=ModelicaMaritime.Constants.small) = 0.025;
  parameter ModelicaMaritime.Types.Angle directions[nComponents] =
    zeros(nComponents);
  parameter Integer phaseSeed(min=0) = 1
    "Deterministic integer seed used by the default phase sequence";
  parameter ModelicaMaritime.Types.Angle phases[nComponents] =
    {6.283185307179586
      * (0.6180339887498949 * index + 0.4142135623730950 * phaseSeed)
      for index in 1:nComponents};
  parameter ModelicaMaritime.Types.Velocity windSpeed = 15;
  parameter ModelicaMaritime.Types.Length significantWaveHeight = 3;
  parameter Real peakPeriod(unit="s", min=ModelicaMaritime.Constants.small) = 9;
  parameter Real peakEnhancement(min=1) = 3.3;
  ModelicaMaritime.Interfaces.Vector3Input positionNED(each unit="m");
  ModelicaMaritime.Interfaces.RealOutput elevation(unit="m");
  ModelicaMaritime.Interfaces.Vector3Output waterVelocityNED(each unit="m/s");
  output Real spectralDensity[nComponents](each unit="m2.s");
  output Real componentAmplitude[nComponents](each unit="m");
  output Real spectralMomentZero(unit="m2");
  output Real realizedSignificantWaveHeight(unit="m");
protected
  Real angularFrequency[nComponents](each unit="rad/s");
  Real waveNumber[nComponents](each unit="1/m");
  Real wavePhase[nComponents];
  Real attenuation[nComponents];
  Real elevationComponent[nComponents](each unit="m");
  Real velocityComponent[nComponents, 3](each unit="m/s");
equation
  for index in 1:nComponents loop
    spectralDensity[index] =
      if spectrumKind == 1 then
        ModelicaMaritime.Environment.Waves.piersonMoskowitzSpectrum(
          frequencies[index],
          windSpeed)
      elseif spectrumKind == 2 then
        ModelicaMaritime.Environment.Waves.jonswapSpectrum(
          frequencies[index],
          significantWaveHeight,
          peakPeriod,
          peakEnhancement)
      else
        ModelicaMaritime.Environment.Waves.bretschneiderSpectrum(
          frequencies[index],
          significantWaveHeight,
          peakPeriod);
    componentAmplitude[index] = sqrt(
      2 * spectralDensity[index] * frequencyWidth);
    angularFrequency[index] = 6.283185307179586 * frequencies[index];
    waveNumber[index] =
      ModelicaMaritime.Environment.Waves.deepWaterWaveNumber(
        angularFrequency[index]);
    wavePhase[index] = waveNumber[index] * (
      cos(directions[index]) * positionNED[1]
      + sin(directions[index]) * positionNED[2])
      - angularFrequency[index] * time + phases[index];
    attenuation[index] = exp(-waveNumber[index] * max(positionNED[3], 0));
    elevationComponent[index] =
      componentAmplitude[index] * cos(wavePhase[index]);
    velocityComponent[index, 1] =
      componentAmplitude[index] * angularFrequency[index] * attenuation[index]
        * cos(wavePhase[index]) * cos(directions[index]);
    velocityComponent[index, 2] =
      componentAmplitude[index] * angularFrequency[index] * attenuation[index]
        * cos(wavePhase[index]) * sin(directions[index]);
    velocityComponent[index, 3] =
      -componentAmplitude[index] * angularFrequency[index] * attenuation[index]
        * sin(wavePhase[index]);
  end for;
  elevation = sum(elevationComponent);
  waterVelocityNED = {
    sum(velocityComponent[:, 1]),
    sum(velocityComponent[:, 2]),
    sum(velocityComponent[:, 3])};
  spectralMomentZero = sum(spectralDensity) * frequencyWidth;
  realizedSignificantWaveHeight = 4 * sqrt(spectralMomentZero);
end DeterministicIrregularWave;
