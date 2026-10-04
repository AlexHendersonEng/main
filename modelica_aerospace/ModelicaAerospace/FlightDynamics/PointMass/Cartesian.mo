within ModelicaAerospace.FlightDynamics.PointMass;
block Cartesian "Cartesian three-degree-of-freedom point mass in NED"
  parameter ModelicaAerospace.Types.Mass mass = 1;
  parameter ModelicaAerospace.Types.Acceleration gravityNED[3] = {0, 0, 9.80665};
  parameter ModelicaAerospace.Types.PointMassInitialState initialState;
  ModelicaAerospace.Interfaces.Vector3Input forceNED(each unit="N");
  ModelicaAerospace.Interfaces.Vector3Output positionNED(each unit="m");
  ModelicaAerospace.Interfaces.Vector3Output velocityNED(each unit="m/s");
  ModelicaAerospace.Interfaces.Vector3Output accelerationNED(each unit="m/s2");
  ModelicaAerospace.Interfaces.RealOutput speed(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput flightPathAngle(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput groundTrack(unit="rad");
protected
  Real positionState[3](start=initialState.positionNED, each fixed=true);
  Real velocityState[3](start=initialState.velocityNED, each fixed=true);
  Real horizontalSpeed;
equation
  assert(mass > 0, "Point-mass mass must be positive");
  der(positionState) = velocityState;
  accelerationNED = forceNED / mass + gravityNED;
  der(velocityState) = accelerationNED;
  positionNED = positionState;
  velocityNED = velocityState;
  speed = sqrt(
    velocityState[1] * velocityState[1]
    + velocityState[2] * velocityState[2]
    + velocityState[3] * velocityState[3]);
  horizontalSpeed = sqrt(
    velocityState[1] * velocityState[1]
    + velocityState[2] * velocityState[2]);
  flightPathAngle = atan2(-velocityState[3], horizontalSpeed);
  groundTrack = atan2(velocityState[2], velocityState[1]);
end Cartesian;
