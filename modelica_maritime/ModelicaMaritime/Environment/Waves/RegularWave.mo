within ModelicaMaritime.Environment.Waves;
block RegularWave "Deep-water Airy wave elevation and velocity"
  parameter ModelicaMaritime.Types.Length amplitude(min=0) = 1;
  parameter Real period(unit="s", min=ModelicaMaritime.Constants.small) = 8;
  parameter ModelicaMaritime.Types.Angle direction = 0
    "Direction toward wave travel, clockwise from north";
  parameter ModelicaMaritime.Types.Angle phase = 0;
  ModelicaMaritime.Interfaces.Vector3Input positionNED(each unit="m");
  ModelicaMaritime.Interfaces.RealOutput elevation(unit="m");
  ModelicaMaritime.Interfaces.Vector3Output waterVelocityNED(each unit="m/s");
  output Real angularFrequency(unit="rad/s");
  output Real waveNumber(unit="1/m");
protected
  Real wavePhase;
  Real attenuation;
equation
  angularFrequency = 6.283185307179586 / period;
  waveNumber = ModelicaMaritime.Environment.Waves.deepWaterWaveNumber(
    angularFrequency);
  wavePhase = waveNumber * (
    cos(direction) * positionNED[1] + sin(direction) * positionNED[2])
    - angularFrequency * time + phase;
  attenuation = exp(-waveNumber * max(positionNED[3], 0));
  elevation = amplitude * cos(wavePhase);
  waterVelocityNED = {
    amplitude * angularFrequency * attenuation * cos(wavePhase) * cos(direction),
    amplitude * angularFrequency * attenuation * cos(wavePhase) * sin(direction),
    -amplitude * angularFrequency * attenuation * sin(wavePhase)};
end RegularWave;
