within ModelicaAerospace.Types;
record PointMassInitialState "Initial Cartesian point-mass state in NED"
  ModelicaAerospace.Types.Length positionNED[3] = {0, 0, 0};
  ModelicaAerospace.Types.Velocity velocityNED[3] = {0, 0, 0};
end PointMassInitialState;
