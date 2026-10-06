within ModelicaMaritime.Navigation;
block KinematicOutputs "Derive marine speed, ground track, depth rate, and sideslip"
  ModelicaMaritime.Interfaces.Vector3Input velocityNED(each unit="m/s");
  ModelicaMaritime.Interfaces.Vector3Input velocityBody(each unit="m/s");
  ModelicaMaritime.Interfaces.RealOutput speed(unit="m/s");
  ModelicaMaritime.Interfaces.RealOutput horizontalSpeed(unit="m/s");
  ModelicaMaritime.Interfaces.RealOutput groundTrack(unit="rad");
  ModelicaMaritime.Interfaces.RealOutput depthRate(unit="m/s");
  ModelicaMaritime.Interfaces.RealOutput sideslip(unit="rad");
equation
  horizontalSpeed = sqrt(
    velocityNED[1] * velocityNED[1] + velocityNED[2] * velocityNED[2]);
  speed = sqrt(horizontalSpeed * horizontalSpeed + velocityNED[3] * velocityNED[3]);
  groundTrack = ModelicaMaritime.Mathematics.wrapHeading(
    atan2(velocityNED[2], velocityNED[1]));
  depthRate = velocityNED[3];
  sideslip = atan2(velocityBody[2], velocityBody[1]);
end KinematicOutputs;
