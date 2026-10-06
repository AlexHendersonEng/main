within ModelicaAutomotive.Types;
record PlanarInitialState "Initial planar vehicle state"
  ModelicaAutomotive.Types.Length positionWorld[2] = {0, 0};
  ModelicaAutomotive.Types.Angle yaw = 0;
  ModelicaAutomotive.Types.Velocity longitudinalVelocity = 0;
  ModelicaAutomotive.Types.Velocity lateralVelocity = 0;
  ModelicaAutomotive.Types.AngularVelocity yawRate = 0;
end PlanarInitialState;
