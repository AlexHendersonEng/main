within ModelicaAerospace.Environment.Wind;
function linearWindShear "Apply a linear wind gradient above a reference altitude"
  input ModelicaAerospace.Types.Velocity referenceWindNED[3];
  input ModelicaAerospace.Types.Length altitude;
  input ModelicaAerospace.Types.Length referenceAltitude = 0;
  input Real gradientNED[3](each unit="1/s") = {0, 0, 0};
  output ModelicaAerospace.Types.Velocity windNED[3];
algorithm
  windNED := referenceWindNED + gradientNED * (altitude - referenceAltitude);
end linearWindShear;
