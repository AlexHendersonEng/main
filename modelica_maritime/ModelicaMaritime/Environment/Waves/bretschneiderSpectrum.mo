within ModelicaMaritime.Environment.Waves;
function bretschneiderSpectrum "Bretschneider spectrum from significant height and peak period"
  input Real frequency(unit="Hz");
  input ModelicaMaritime.Types.Length significantWaveHeight;
  input Real peakPeriod(unit="s");
  output Real spectralDensity(unit="m2.s");
protected
  Real peakFrequency;
algorithm
  assert(frequency > 0, "Spectrum frequency must be positive");
  assert(significantWaveHeight >= 0, "Significant wave height must be non-negative");
  assert(peakPeriod > 0, "Peak period must be positive");
  peakFrequency := 1 / peakPeriod;
  spectralDensity := 0.3125 * significantWaveHeight * significantWaveHeight
    * peakFrequency ^ 4 / frequency ^ 5
    * exp(-1.25 * (peakFrequency / frequency) ^ 4);
end bretschneiderSpectrum;
