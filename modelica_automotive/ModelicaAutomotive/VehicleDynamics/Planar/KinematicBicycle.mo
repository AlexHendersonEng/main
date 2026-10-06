within ModelicaAutomotive.VehicleDynamics.Planar;
block KinematicBicycle "Rear-axle-reference kinematic bicycle model"
  parameter ModelicaAutomotive.Types.VehicleGeometry geometry;
  parameter ModelicaAutomotive.Types.PlanarInitialState initialState;
  ModelicaAutomotive.Interfaces.RealInput speed(unit="m/s");
  ModelicaAutomotive.Interfaces.RealInput steeringAngle(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput positionX(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput positionY(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput yaw(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput yawRate(unit="rad/s");
protected
  Real positionXState(start=initialState.positionWorld[1], fixed=true, unit="m");
  Real positionYState(start=initialState.positionWorld[2], fixed=true, unit="m");
  Real yawState(start=initialState.yaw, fixed=true, unit="rad");
equation
  assert(geometry.wheelbase > 0, "wheelbase must be positive");
  yawRate = speed * tan(steeringAngle) / geometry.wheelbase;
  der(positionXState) = speed * cos(yawState);
  der(positionYState) = speed * sin(yawState);
  der(yawState) = yawRate;
  positionX = positionXState;
  positionY = positionYState;
  yaw = yawState;
end KinematicBicycle;
