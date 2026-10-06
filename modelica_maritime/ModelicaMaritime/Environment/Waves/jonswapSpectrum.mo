within ModelicaMaritime.Environment.Waves;
function jonswapSpectrum "JONSWAP spectrum with significant-height normalization"
  input Real frequency(unit="Hz");
  input ModelicaMaritime.Types.Length significantWaveHeight;
  input Real peakPeriod(unit="s");
  input Real peakEnhancement(min=1) = 3.3;
  output Real spectralDensity(unit="m2.s");
protected
  Real peakFrequency;
  Real sigma;
  Real peakShape;
  Real alpha;
algorithm
  assert(frequency > 0, "Spectrum frequency must be positive");
  assert(significantWaveHeight >= 0, "Significant wave height must be non-negative");
  assert(peakPeriod > 0, "Peak period must be positive");
  assert(peakEnhancement >= 1, "JONSWAP peak enhancement must be at least one");
  peakFrequency := 1 / peakPeriod;
  sigma := if frequency <= peakFrequency then 0.07 else 0.09;
  peakShape := exp(
    -0.5 * ((frequency - peakFrequency) / (sigma * peakFrequency)) ^ 2);
  alpha := 5.061 * significantWaveHeight * significantWaveHeight
    / peakPeriod ^ 4 * (1 - 0.287 * log(peakEnhancement));
  spectralDensity := alpha * ModelicaMaritime.Constants.standardGravity ^ 2
    / (6.283185307179586 ^ 4 * frequency ^ 5)
    * exp(-1.25 * (peakFrequency / frequency) ^ 4)
    * peakEnhancement ^ peakShape;
end jonswapSpectrum;
