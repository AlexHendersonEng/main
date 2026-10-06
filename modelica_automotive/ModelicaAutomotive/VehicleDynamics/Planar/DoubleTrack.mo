within ModelicaAutomotive.VehicleDynamics.Planar;
block DoubleTrack "Force-driven planar double-track rigid vehicle model"
  parameter ModelicaAutomotive.Types.Mass mass = 1500;
  parameter ModelicaAutomotive.Types.Inertia yawInertia = 2500;
  parameter ModelicaAutomotive.Types.VehicleGeometry geometry;
  parameter ModelicaAutomotive.Types.PlanarInitialState initialState;
  parameter ModelicaAutomotive.Types.Acceleration gravity =
    ModelicaAutomotive.Constants.standardGravity;
  ModelicaAutomotive.Interfaces.CornerInput tireLongitudinalForce(each unit="N");
  ModelicaAutomotive.Interfaces.CornerInput tireLateralForce(each unit="N");
  ModelicaAutomotive.Interfaces.CornerInput steeringAngle(each unit="rad");
  ModelicaAutomotive.Interfaces.RealInput externalLongitudinalForce(unit="N");
  ModelicaAutomotive.Interfaces.RealInput externalLateralForce(unit="N");
  ModelicaAutomotive.Interfaces.RealInput externalYawMoment(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput positionX(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput positionY(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput yaw(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput longitudinalVelocity(unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput lateralVelocity(unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput yawRate(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput longitudinalAcceleration(unit="m/s2");
  ModelicaAutomotive.Interfaces.RealOutput lateralAcceleration(unit="m/s2");
  ModelicaAutomotive.Interfaces.RealOutput totalLongitudinalForce(unit="N");
  ModelicaAutomotive.Interfaces.RealOutput totalLateralForce(unit="N");
  ModelicaAutomotive.Interfaces.RealOutput totalYawMoment(unit="N.m");
  ModelicaAutomotive.Interfaces.CornerOutput normalLoad(each unit="N");
protected
  parameter Real frontDistance(unit="m") =
    geometry.centerOfMassToFrontAxle;
  parameter Real rearDistance(unit="m") =
    geometry.wheelbase - geometry.centerOfMassToFrontAxle;
  Real positionXState(start=initialState.positionWorld[1], fixed=true, unit="m");
  Real positionYState(start=initialState.positionWorld[2], fixed=true, unit="m");
  Real yawState(start=initialState.yaw, fixed=true, unit="rad");
  Real longitudinalVelocityState(
    start=initialState.longitudinalVelocity,
    fixed=true,
    unit="m/s");
  Real lateralVelocityState(
    start=initialState.lateralVelocity,
    fixed=true,
    unit="m/s");
  Real yawRateState(start=initialState.yawRate, fixed=true, unit="rad/s");
  Real forceXBody[4](each unit="N");
  Real forceYBody[4](each unit="N");
  Real wheelPositionX[4](each unit="m");
  Real wheelPositionY[4](each unit="m");
  Real frontAxleLoad(unit="N");
  Real rearAxleLoad(unit="N");
  Real frontLateralTransfer(unit="N");
  Real rearLateralTransfer(unit="N");
equation
  assert(mass > 0, "Vehicle mass must be positive");
  assert(yawInertia > 0, "Vehicle yaw inertia must be positive");
  assert(geometry.wheelbase > 0, "wheelbase must be positive");
  assert(frontDistance > 0 and rearDistance > 0,
    "Center of mass must lie between the axles");
  assert(geometry.frontTrack > 0 and geometry.rearTrack > 0,
    "Track widths must be positive");
  assert(geometry.centerOfMassHeight >= 0,
    "centerOfMassHeight must not be negative");
  wheelPositionX = {
    frontDistance,
    frontDistance,
    -rearDistance,
    -rearDistance};
  wheelPositionY = {
    geometry.frontTrack / 2,
    -geometry.frontTrack / 2,
    geometry.rearTrack / 2,
    -geometry.rearTrack / 2};
  for corner in 1:4 loop
    forceXBody[corner] =
      cos(steeringAngle[corner]) * tireLongitudinalForce[corner]
      - sin(steeringAngle[corner]) * tireLateralForce[corner];
    forceYBody[corner] =
      sin(steeringAngle[corner]) * tireLongitudinalForce[corner]
      + cos(steeringAngle[corner]) * tireLateralForce[corner];
  end for;
  totalLongitudinalForce = sum(forceXBody) + externalLongitudinalForce;
  totalLateralForce = sum(forceYBody) + externalLateralForce;
  totalYawMoment = sum(
    wheelPositionX .* forceYBody - wheelPositionY .* forceXBody)
    + externalYawMoment;
  longitudinalAcceleration =
    totalLongitudinalForce / mass
    + lateralVelocityState * yawRateState;
  lateralAcceleration =
    totalLateralForce / mass
    - longitudinalVelocityState * yawRateState;
  der(longitudinalVelocityState) = longitudinalAcceleration;
  der(lateralVelocityState) = lateralAcceleration;
  der(yawRateState) = totalYawMoment / yawInertia;
  der(yawState) = yawRateState;
  der(positionXState) =
    longitudinalVelocityState * cos(yawState)
    - lateralVelocityState * sin(yawState);
  der(positionYState) =
    longitudinalVelocityState * sin(yawState)
    + lateralVelocityState * cos(yawState);
  frontAxleLoad = mass * gravity * rearDistance / geometry.wheelbase
    - mass * longitudinalAcceleration * geometry.centerOfMassHeight
      / geometry.wheelbase;
  rearAxleLoad = mass * gravity * frontDistance / geometry.wheelbase
    + mass * longitudinalAcceleration * geometry.centerOfMassHeight
      / geometry.wheelbase;
  frontLateralTransfer =
    mass * lateralAcceleration * geometry.centerOfMassHeight * rearDistance
    / (geometry.wheelbase * geometry.frontTrack);
  rearLateralTransfer =
    mass * lateralAcceleration * geometry.centerOfMassHeight * frontDistance
    / (geometry.wheelbase * geometry.rearTrack);
  normalLoad[ModelicaAutomotive.Constants.frontLeft] =
    frontAxleLoad / 2 - frontLateralTransfer;
  normalLoad[ModelicaAutomotive.Constants.frontRight] =
    frontAxleLoad / 2 + frontLateralTransfer;
  normalLoad[ModelicaAutomotive.Constants.rearLeft] =
    rearAxleLoad / 2 - rearLateralTransfer;
  normalLoad[ModelicaAutomotive.Constants.rearRight] =
    rearAxleLoad / 2 + rearLateralTransfer;
  positionX = positionXState;
  positionY = positionYState;
  yaw = yawState;
  longitudinalVelocity = longitudinalVelocityState;
  lateralVelocity = lateralVelocityState;
  yawRate = yawRateState;
end DoubleTrack;
