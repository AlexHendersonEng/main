within ModelicaAutomotive.VehicleDynamics.Planar;
block DynamicBicycle "Linear-tire dynamic single-track vehicle model"
  parameter ModelicaAutomotive.Types.PlanarVehicleParameters vehicle;
  parameter ModelicaAutomotive.Types.PlanarInitialState initialState;
  parameter ModelicaAutomotive.Types.Velocity velocityRegularization = 0.1;
  ModelicaAutomotive.Interfaces.RealInput longitudinalVelocity(unit="m/s");
  ModelicaAutomotive.Interfaces.RealInput steeringAngle(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput positionX(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput positionY(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput yaw(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput lateralVelocity(unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput yawRate(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput lateralAcceleration(unit="m/s2");
  ModelicaAutomotive.Interfaces.RealOutput frontSlipAngle(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput rearSlipAngle(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput frontLateralForce(unit="N");
  ModelicaAutomotive.Interfaces.RealOutput rearLateralForce(unit="N");
protected
  parameter Real rearDistance(unit="m") =
    vehicle.geometry.wheelbase - vehicle.geometry.centerOfMassToFrontAxle;
  Real positionXState(start=initialState.positionWorld[1], fixed=true, unit="m");
  Real positionYState(start=initialState.positionWorld[2], fixed=true, unit="m");
  Real yawState(start=initialState.yaw, fixed=true, unit="rad");
  Real lateralVelocityState(
    start=initialState.lateralVelocity,
    fixed=true,
    unit="m/s");
  Real yawRateState(start=initialState.yawRate, fixed=true, unit="rad/s");
  Real referenceSpeed(unit="m/s");
  Real lateralVelocityDerivative(unit="m/s2");
equation
  assert(vehicle.mass > 0, "Vehicle mass must be positive");
  assert(vehicle.yawInertia > 0, "Vehicle yaw inertia must be positive");
  assert(vehicle.geometry.wheelbase > 0, "wheelbase must be positive");
  assert(vehicle.geometry.centerOfMassToFrontAxle > 0,
    "centerOfMassToFrontAxle must be positive");
  assert(rearDistance > 0, "Center of mass must lie between the axles");
  assert(vehicle.frontCorneringStiffness > 0,
    "frontCorneringStiffness must be positive");
  assert(vehicle.rearCorneringStiffness > 0,
    "rearCorneringStiffness must be positive");
  assert(velocityRegularization > 0, "velocityRegularization must be positive");
  referenceSpeed = sqrt(
    longitudinalVelocity * longitudinalVelocity
    + velocityRegularization * velocityRegularization);
  frontSlipAngle = steeringAngle - atan2(
    lateralVelocityState
      + vehicle.geometry.centerOfMassToFrontAxle * yawRateState,
    referenceSpeed);
  rearSlipAngle = -atan2(
    lateralVelocityState - rearDistance * yawRateState,
    referenceSpeed);
  frontLateralForce =
    vehicle.frontCorneringStiffness * frontSlipAngle;
  rearLateralForce = vehicle.rearCorneringStiffness * rearSlipAngle;
  lateralVelocityDerivative =
    (frontLateralForce * cos(steeringAngle) + rearLateralForce) / vehicle.mass
    - longitudinalVelocity * yawRateState;
  der(lateralVelocityState) = lateralVelocityDerivative;
  der(yawRateState) = (
    vehicle.geometry.centerOfMassToFrontAxle
      * frontLateralForce * cos(steeringAngle)
    - rearDistance * rearLateralForce) / vehicle.yawInertia;
  der(yawState) = yawRateState;
  der(positionXState) =
    longitudinalVelocity * cos(yawState) - lateralVelocityState * sin(yawState);
  der(positionYState) =
    longitudinalVelocity * sin(yawState) + lateralVelocityState * cos(yawState);
  lateralAcceleration =
    lateralVelocityDerivative + longitudinalVelocity * yawRateState;
  positionX = positionXState;
  positionY = positionYState;
  yaw = yawState;
  lateralVelocity = lateralVelocityState;
  yawRate = yawRateState;
end DynamicBicycle;
