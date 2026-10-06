within ModelicaMaritime.Types;
record PlanarMassProperties "Rigid-body properties for surge, sway, and yaw"
  ModelicaMaritime.Types.Mass mass = 1 "Vehicle mass";
  ModelicaMaritime.Types.Length centerOfGravityX = 0
    "Longitudinal center of gravity from the body reference point";
  ModelicaMaritime.Types.Inertia yawInertia = 1
    "Yaw inertia about the center of gravity";
end PlanarMassProperties;
