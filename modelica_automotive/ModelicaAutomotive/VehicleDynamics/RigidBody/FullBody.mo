within ModelicaAutomotive.VehicleDynamics.RigidBody;
block FullBody "Flat-world body-axis six-degree-of-freedom vehicle dynamics"
  extends ModelicaAutomotive.Interfaces.PartialVehicleDynamics;
  parameter ModelicaAutomotive.Types.MassProperties massProperties;
  parameter ModelicaAutomotive.Types.VehicleInitialState initialState;
  parameter ModelicaAutomotive.Types.Acceleration gravity =
    ModelicaAutomotive.Constants.standardGravity;
  parameter Real quaternionStabilization(unit="1/s") = 20;
  ModelicaAutomotive.Interfaces.Vector3Output velocityWorld(each unit="m/s");
  ModelicaAutomotive.Interfaces.Vector3Output accelerationBody(each unit="m/s2");
  ModelicaAutomotive.Interfaces.Vector3Output angularAccelerationBody(each unit="rad/s2");
  ModelicaAutomotive.Interfaces.RealOutput quaternionNorm;
protected
  Real positionState[3](start=initialState.positionWorld, each fixed=true);
  Real velocityState[3](start=initialState.velocityBody, each fixed=true);
  Real quaternionState[4](
    start=initialState.quaternionBodyToWorld,
    each fixed=true);
  Real angularVelocityState[3](
    start=initialState.angularVelocityBody,
    each fixed=true);
  Real dcmBodyToWorld[3, 3];
  Real gravityBody[3];
  Real angularMomentumBody[3];
  Real gyroscopicMoment[3];
  Real quaternionRate[4];
equation
  assert(massProperties.mass > 0, "Rigid-body mass must be positive");
  assert(gravity > 0, "gravity must be positive");
  assert(quaternionStabilization >= 0,
    "quaternionStabilization must not be negative");
  dcmBodyToWorld =
    ModelicaAutomotive.Mathematics.quaternionToDCM(quaternionState);
  velocityWorld = dcmBodyToWorld * velocityState;
  gravityBody = transpose(dcmBodyToWorld) * {0, 0, -gravity};
  accelerationBody = forceBody / massProperties.mass + gravityBody
    - ModelicaAutomotive.Mathematics.cross3(
      angularVelocityState,
      velocityState);
  der(positionState) = velocityWorld;
  der(velocityState) = accelerationBody;
  angularMomentumBody = massProperties.inertiaBody * angularVelocityState;
  gyroscopicMoment = ModelicaAutomotive.Mathematics.cross3(
    angularVelocityState,
    angularMomentumBody);
  angularAccelerationBody = ModelicaAutomotive.Mathematics.solveLinear3(
    massProperties.inertiaBody,
    momentBody - gyroscopicMoment);
  der(angularVelocityState) = angularAccelerationBody;
  quaternionNorm = sqrt(
    quaternionState[1] * quaternionState[1]
    + quaternionState[2] * quaternionState[2]
    + quaternionState[3] * quaternionState[3]
    + quaternionState[4] * quaternionState[4]);
  quaternionRate = ModelicaAutomotive.Mathematics.quaternionDerivative(
    quaternionState,
    angularVelocityState);
  der(quaternionState) = quaternionRate
    + quaternionStabilization * (1 - quaternionNorm * quaternionNorm)
      * quaternionState;
  positionWorld = positionState;
  velocityBody = velocityState;
  quaternionBodyToWorld = quaternionState / quaternionNorm;
  angularVelocityBody = angularVelocityState;
end FullBody;
