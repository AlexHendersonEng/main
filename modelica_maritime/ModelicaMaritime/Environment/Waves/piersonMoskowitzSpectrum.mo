within ModelicaMaritime.Environment.Waves;
function piersonMoskowitzSpectrum "Pierson-Moskowitz frequency spectrum from wind speed"
  input Real frequency(unit="Hz");
  input ModelicaMaritime.Types.Velocity windSpeed;
  output Real spectralDensity(unit="m2.s");
protected
  Real gravity = ModelicaMaritime.Constants.standardGravity;
  Real angularScale;
algorithm
  assert(frequency > 0, "Spectrum frequency must be positive");
  assert(windSpeed > 0, "Pierson-Moskowitz wind speed must be positive");
  angularScale := gravity / (6.283185307179586 * windSpeed * frequency);
  spectralDensity := 0.0081 * gravity * gravity
    / (6.283185307179586 ^ 4 * frequency ^ 5)
    * exp(-0.74 * angularScale ^ 4);
end piersonMoskowitzSpectrum;
