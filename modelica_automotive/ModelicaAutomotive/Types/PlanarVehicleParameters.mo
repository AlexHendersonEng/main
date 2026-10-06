within ModelicaAutomotive.Types;
record PlanarVehicleParameters "Planar passenger-vehicle parameters"
  ModelicaAutomotive.Types.Mass mass = 1500;
  ModelicaAutomotive.Types.Inertia yawInertia = 2500;
  ModelicaAutomotive.Types.VehicleGeometry geometry;
  Real frontCorneringStiffness(unit="N/rad") = 100000;
  Real rearCorneringStiffness(unit="N/rad") = 120000;
end PlanarVehicleParameters;
