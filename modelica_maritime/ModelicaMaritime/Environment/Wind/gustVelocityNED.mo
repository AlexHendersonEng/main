within ModelicaMaritime.Environment.Wind;
function gustVelocityNED "Add deterministic sinusoidal gust components"
  input Real sampleTime(unit="s");
  input ModelicaMaritime.Types.Velocity meanVelocityNED[3];
  input ModelicaMaritime.Types.Velocity gustAmplitudeNED[3];
  input Real gustFrequency(unit="Hz", min=0);
  input ModelicaMaritime.Types.Angle phase = 0;
  output ModelicaMaritime.Types.Velocity velocityNED[3];
algorithm
  velocityNED := meanVelocityNED
    + gustAmplitudeNED
      * sin(6.283185307179586 * gustFrequency * sampleTime + phase);
end gustVelocityNED;
