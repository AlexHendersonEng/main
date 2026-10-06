within ModelicaMaritime.Environment.Waves;
function deepWaterWaveNumber "Deep-water dispersion relation k = omega^2/g"
  input Real angularFrequency(unit="rad/s");
  output Real waveNumber(unit="1/m");
algorithm
  assert(angularFrequency > 0, "Wave angular frequency must be positive");
  waveNumber := angularFrequency * angularFrequency
    / ModelicaMaritime.Constants.standardGravity;
end deepWaterWaveNumber;
