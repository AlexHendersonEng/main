within ModelicaAerospace.Types;
record PropulsionCommand "Normalized propulsion commands"
  Real throttle(min=0, max=1) = 0;
  Real mixture(min=0, max=1) = 1;
  Boolean enabled = true;
end PropulsionCommand;
