within ModelicaMaritime.Types;
record MassProperties "Vehicle mass and buoyancy properties expressed in body axes"
  ModelicaMaritime.Types.Mass mass = 1 "Vehicle mass";
  ModelicaMaritime.Types.Length centerOfGravityBody[3] = {0, 0, 0}
    "Center of gravity from the body reference point";
  ModelicaMaritime.Types.Length centerOfBuoyancyBody[3] = {0, 0, 0}
    "Center of buoyancy from the body reference point";
  ModelicaMaritime.Types.Inertia inertiaBody[3, 3] = [1, 0, 0; 0, 1, 0; 0, 0, 1]
    "Symmetric inertia tensor about the center of gravity";
  Real displacedVolume(unit="m3", min=0) = 0 "Displaced fluid volume";
end MassProperties;
