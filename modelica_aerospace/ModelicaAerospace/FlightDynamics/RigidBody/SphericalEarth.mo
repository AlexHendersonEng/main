within ModelicaAerospace.FlightDynamics.RigidBody;
block SphericalEarth "Rotating spherical-Earth ECEF six-degree-of-freedom dynamics"
  parameter ModelicaAerospace.Types.MassProperties massProperties;
  parameter ModelicaAerospace.Types.SphericalInitialState initialState;
  parameter Real gravitationalParameter(unit="m3/s2") =
    ModelicaAerospace.Constants.WGS84.gravitationalParameter;
  parameter ModelicaAerospace.Types.AngularVelocity earthRotationRate =
    ModelicaAerospace.Constants.WGS84.earthRotationRate;
  parameter Real quaternionStabilization(unit="1/s") = 20;
  ModelicaAerospace.Interfaces.Vector3Input forceBody(each unit="N");
  ModelicaAerospace.Interfaces.Vector3Input momentBody(each unit="N.m");
  ModelicaAerospace.Interfaces.Vector3Output positionECEF(each unit="m");
  ModelicaAerospace.Interfaces.Vector3Output velocityECEF(each unit="m/s");
  ModelicaAerospace.Interfaces.QuaternionOutput quaternionBodyToECEF;
  ModelicaAerospace.Interfaces.Vector3Output angularVelocityBody(each unit="rad/s");
  ModelicaAerospace.Interfaces.Vector3Output velocityBody(each unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput latitude(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput longitude(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput altitude(unit="m");
  ModelicaAerospace.Interfaces.RealOutput airspeed(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput angleOfAttack(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput sideslip(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput specificMechanicalEnergy(unit="m2/s2");
  ModelicaAerospace.Interfaces.Vector3Output specificAngularMomentum(each unit="m2/s");
  ModelicaAerospace.Interfaces.RealOutput quaternionNorm;
protected
  Real positionState[3](start=initialState.positionECEF, each fixed=true);
  Real velocityState[3](start=initialState.velocityECEF, each fixed=true);
  Real quaternionState[4](start=initialState.quaternionBodyToECEF, each fixed=true);
  Real angularVelocityState[3](
    start=initialState.angularVelocityBody,
    each fixed=true);
  Real dcmBodyToECEF[3, 3];
  Real radius;
  Real forceECEF[3];
  Real gravityECEF[3];
  Real earthRateECEF[3];
  Real transportVelocity[3];
  Real coriolisAcceleration[3];
  Real centrifugalAcceleration[3];
  Real inertialVelocity[3];
  Real bodyEarthRate[3];
  Real relativeAngularVelocity[3];
  Real angularMomentumBody[3];
  Real gyroscopicMoment[3];
  Real angularAccelerationBody[3];
  Real quaternionRate[4];
  ModelicaAerospace.Types.GeodeticPosition geodetic;
equation
  assert(massProperties.mass > 0, "Rigid-body mass must be positive");
  radius = sqrt(
    positionState[1] * positionState[1]
    + positionState[2] * positionState[2]
    + positionState[3] * positionState[3]);
  assert(radius > 1e-12, "ECEF radius must be positive");
  dcmBodyToECEF = ModelicaAerospace.Mathematics.quaternionToDCM(quaternionState);
  forceECEF = dcmBodyToECEF * forceBody / massProperties.mass;
  gravityECEF = -gravitationalParameter * positionState / (radius * radius * radius);
  earthRateECEF = {0, 0, earthRotationRate};
  transportVelocity =
    ModelicaAerospace.Mathematics.cross3(earthRateECEF, positionState);
  coriolisAcceleration =
    -2 * ModelicaAerospace.Mathematics.cross3(earthRateECEF, velocityState);
  centrifugalAcceleration =
    -ModelicaAerospace.Mathematics.cross3(earthRateECEF, transportVelocity);
  der(positionState) = velocityState;
  der(velocityState) =
    forceECEF + gravityECEF + coriolisAcceleration + centrifugalAcceleration;
  angularMomentumBody = massProperties.inertiaBody * angularVelocityState;
  gyroscopicMoment =
    ModelicaAerospace.Mathematics.cross3(angularVelocityState, angularMomentumBody);
  angularAccelerationBody = ModelicaAerospace.Mathematics.solveLinear3(
    massProperties.inertiaBody,
    momentBody - gyroscopicMoment);
  der(angularVelocityState) = angularAccelerationBody;
  bodyEarthRate = transpose(dcmBodyToECEF) * earthRateECEF;
  relativeAngularVelocity = angularVelocityState - bodyEarthRate;
  quaternionNorm = sqrt(
    quaternionState[1] * quaternionState[1]
    + quaternionState[2] * quaternionState[2]
    + quaternionState[3] * quaternionState[3]
    + quaternionState[4] * quaternionState[4]);
  quaternionRate = ModelicaAerospace.Mathematics.quaternionDerivative(
    quaternionState,
    relativeAngularVelocity);
  der(quaternionState) = quaternionRate
    + quaternionStabilization * (1 - quaternionNorm * quaternionNorm) * quaternionState;
  positionECEF = positionState;
  velocityECEF = velocityState;
  quaternionBodyToECEF = quaternionState / quaternionNorm;
  angularVelocityBody = angularVelocityState;
  velocityBody = transpose(dcmBodyToECEF) * velocityState;
  geodetic = ModelicaAerospace.Coordinates.ecefToGeodetic(positionState);
  latitude = geodetic.latitude;
  longitude = geodetic.longitude;
  altitude = geodetic.altitude;
  airspeed = sqrt(
    velocityBody[1] * velocityBody[1]
    + velocityBody[2] * velocityBody[2]
    + velocityBody[3] * velocityBody[3]);
  angleOfAttack = atan2(velocityBody[3], velocityBody[1]);
  sideslip = if airspeed > ModelicaAerospace.Constants.Numerics.small then
    asin(ModelicaAerospace.Mathematics.clamp(velocityBody[2] / airspeed, -1, 1))
    else 0;
  inertialVelocity = velocityState + transportVelocity;
  specificMechanicalEnergy = 0.5 * (
    inertialVelocity[1] * inertialVelocity[1]
    + inertialVelocity[2] * inertialVelocity[2]
    + inertialVelocity[3] * inertialVelocity[3])
    - gravitationalParameter / radius;
  specificAngularMomentum =
    ModelicaAerospace.Mathematics.cross3(positionState, inertialVelocity);
end SphericalEarth;
