within ModelicaAutomotive.Types;
record MassProperties "Vehicle mass properties expressed in body axes"
  ModelicaAutomotive.Types.Mass mass = 1500;
  ModelicaAutomotive.Types.Length centerOfMassBody[3] = {0, 0, 0};
  ModelicaAutomotive.Types.Inertia inertiaBody[3, 3] =
    [600, 0, 0; 0, 1800, 0; 0, 0, 2000];
end MassProperties;
