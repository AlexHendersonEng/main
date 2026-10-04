within ModelicaAerospace.Types;
record TranslationalState "Position and velocity expressed in an explicitly documented frame"
  ModelicaAerospace.Types.Length position[3] = {0, 0, 0} "Position coordinates";
  ModelicaAerospace.Types.Velocity velocity[3] = {0, 0, 0} "Velocity components";
end TranslationalState;
