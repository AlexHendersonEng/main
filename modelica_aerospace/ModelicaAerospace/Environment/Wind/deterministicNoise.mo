within ModelicaAerospace.Environment.Wind;
function deterministicNoise "Deterministic unit-RMS broadband forcing"
  input Real elapsedTime(unit="s");
  input Integer seed = 1;
  input Integer sequence = 1;
  output Real value;
protected
  Real phase;
algorithm
  value := 0;
  for index in 1:8 loop
    phase := 0.754877666 * seed * (index + sequence);
    value := value + sin(
      (0.37 * index + 0.11 * sequence + 0.013 * seed) * elapsedTime + phase);
  end for;
  value := value / 2;
end deterministicNoise;
