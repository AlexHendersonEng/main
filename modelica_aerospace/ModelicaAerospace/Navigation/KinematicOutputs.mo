within ModelicaAerospace.Navigation;
block KinematicOutputs "Derive speed, track, and flight-path angle from NED velocity"
  ModelicaAerospace.Interfaces.Vector3Input velocityNED(each unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput speed(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput groundSpeed(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput groundTrack(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput flightPathAngle(unit="rad");
equation
  groundSpeed = sqrt(
    velocityNED[1] * velocityNED[1] + velocityNED[2] * velocityNED[2]);
  speed = sqrt(groundSpeed * groundSpeed + velocityNED[3] * velocityNED[3]);
  groundTrack = atan2(velocityNED[2], velocityNED[1]);
  flightPathAngle = atan2(-velocityNED[3], groundSpeed);
end KinematicOutputs;
