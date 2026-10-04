within ModelicaAerospace.Environment.Wind.Blocks;
block OneMinusCosineGust "Finite one-minus-cosine gust in NED coordinates"
  parameter Real startTime(unit="s") = 0;
  parameter Real duration(unit="s") = 1;
  parameter ModelicaAerospace.Types.Velocity amplitudeNED[3] = {0, 0, 0};
  ModelicaAerospace.Interfaces.Vector3Output gustNED(each unit="m/s");
equation
  gustNED = ModelicaAerospace.Environment.Wind.oneMinusCosineGust(
    time,
    startTime,
    duration,
    amplitudeNED);
end OneMinusCosineGust;
