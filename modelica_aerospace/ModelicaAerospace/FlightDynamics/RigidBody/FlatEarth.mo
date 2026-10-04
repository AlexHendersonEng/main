within ModelicaAerospace.FlightDynamics.RigidBody;
block FlatEarth "Flat-Earth body-axis six-degree-of-freedom dynamics"
  extends ModelicaAerospace.Interfaces.PartialVehicleDynamics;
  parameter ModelicaAerospace.Types.MassProperties massProperties;
  parameter ModelicaAerospace.Types.RigidBodyInitialState initialState;
  parameter ModelicaAerospace.Types.Acceleration gravity = 9.80665;
  parameter Real quaternionStabilization(unit="1/s") = 20;
  ModelicaAerospace.Interfaces.Vector3Output velocityNED(each unit="m/s");
  ModelicaAerospace.Interfaces.Vector3Output accelerationBody(each unit="m/s2");
  ModelicaAerospace.Interfaces.Vector3Output angularAccelerationBody(each unit="rad/s2");
  ModelicaAerospace.Interfaces.Vector3Output euler321(each unit="rad");
  ModelicaAerospace.Interfaces.RealOutput airspeed(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput angleOfAttack(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput sideslip(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput flightPathAngle(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput groundTrack(unit="rad");
  ModelicaAerospace.Interfaces.Vector3Output loadFactorBody;
  ModelicaAerospace.Interfaces.RealOutput quaternionNorm;
protected
  Real positionState[3](start=initialState.positionNED, each fixed=true);
  Real velocityState[3](start=initialState.velocityBody, each fixed=true);
  Real quaternionState[4](start=initialState.quaternionBodyToNED, each fixed=true);
  Real angularVelocityState[3](
    start=initialState.angularVelocityBody,
    each fixed=true);
  Real dcmBodyToNED[3, 3];
  Real gravityBody[3];
  Real angularMomentumBody[3];
  Real gyroscopicMoment[3];
  Real quaternionRate[4];
  Real horizontalSpeed;
equation
  assert(massProperties.mass > 0, "Rigid-body mass must be positive");
  dcmBodyToNED = ModelicaAerospace.Mathematics.quaternionToDCM(quaternionState);
  velocityNED = dcmBodyToNED * velocityState;
  gravityBody = transpose(dcmBodyToNED) * {0, 0, gravity};
  accelerationBody = forceBody / massProperties.mass + gravityBody
    - ModelicaAerospace.Mathematics.cross3(angularVelocityState, velocityState);
  der(positionState) = velocityNED;
  der(velocityState) = accelerationBody;
  angularMomentumBody = massProperties.inertiaBody * angularVelocityState;
  gyroscopicMoment =
    ModelicaAerospace.Mathematics.cross3(angularVelocityState, angularMomentumBody);
  angularAccelerationBody = ModelicaAerospace.Mathematics.solveLinear3(
    massProperties.inertiaBody,
    momentBody - gyroscopicMoment);
  der(angularVelocityState) = angularAccelerationBody;
  quaternionNorm = sqrt(
    quaternionState[1] * quaternionState[1]
    + quaternionState[2] * quaternionState[2]
    + quaternionState[3] * quaternionState[3]
    + quaternionState[4] * quaternionState[4]);
  quaternionRate = ModelicaAerospace.Mathematics.quaternionDerivative(
    quaternionState,
    angularVelocityState);
  der(quaternionState) = quaternionRate
    + quaternionStabilization * (1 - quaternionNorm * quaternionNorm) * quaternionState;
  positionNED = positionState;
  velocityBody = velocityState;
  quaternionBodyToNED = quaternionState / quaternionNorm;
  angularVelocityBody = angularVelocityState;
  euler321 =
    ModelicaAerospace.Mathematics.quaternionToEuler321(quaternionBodyToNED);
  airspeed = sqrt(
    velocityState[1] * velocityState[1]
    + velocityState[2] * velocityState[2]
    + velocityState[3] * velocityState[3]);
  angleOfAttack = atan2(velocityState[3], velocityState[1]);
  sideslip = if airspeed > ModelicaAerospace.Constants.Numerics.small then
    asin(ModelicaAerospace.Mathematics.clamp(velocityState[2] / airspeed, -1, 1))
    else 0;
  horizontalSpeed = sqrt(
    velocityNED[1] * velocityNED[1] + velocityNED[2] * velocityNED[2]);
  flightPathAngle = atan2(-velocityNED[3], horizontalSpeed);
  groundTrack = atan2(velocityNED[2], velocityNED[1]);
  loadFactorBody = forceBody / (
    massProperties.mass * ModelicaAerospace.Constants.StandardAtmosphere.standardGravity);
end FlatEarth;
