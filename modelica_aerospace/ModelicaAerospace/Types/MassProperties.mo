within ModelicaAerospace.Types;
record MassProperties "Vehicle mass properties expressed in body axes"
  ModelicaAerospace.Types.Mass mass = 1 "Vehicle mass";
  ModelicaAerospace.Types.Length centerOfMassBody[3] = {0, 0, 0}
    "Center of mass from the body reference point";
  ModelicaAerospace.Types.Inertia inertiaBody[3, 3] = [1, 0, 0; 0, 1, 0; 0, 0, 1]
    "Symmetric inertia tensor about the center of mass";
end MassProperties;
