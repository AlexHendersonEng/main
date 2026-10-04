within ModelicaAerospace.Environment.Wind;
function oneMinusCosineGust "Evaluate a finite one-minus-cosine gust"
  input Real elapsedTime(unit="s");
  input Real startTime(unit="s") = 0;
  input Real duration(unit="s");
  input ModelicaAerospace.Types.Velocity amplitudeNED[3];
  output ModelicaAerospace.Types.Velocity gustNED[3];
protected
  Real phase;
algorithm
  assert(duration > 0, "Gust duration must be positive");
  if elapsedTime < startTime or elapsedTime > startTime + duration then
    gustNED := {0, 0, 0};
  else
    phase := (elapsedTime - startTime) / duration;
    gustNED := 0.5 * (1 - cos(6.283185307179586 * phase)) * amplitudeNED;
  end if;
end oneMinusCosineGust;
