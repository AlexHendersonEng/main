within ModelicaMaritime.Guidance;
block CommandErrors "Wrapped heading plus depth and speed tracking errors"
  ModelicaMaritime.Interfaces.RealInput commandedHeading(unit="rad");
  ModelicaMaritime.Interfaces.RealInput heading(unit="rad");
  ModelicaMaritime.Interfaces.RealInput commandedDepth(unit="m");
  ModelicaMaritime.Interfaces.RealInput depth(unit="m");
  ModelicaMaritime.Interfaces.RealInput commandedSpeed(unit="m/s");
  ModelicaMaritime.Interfaces.RealInput speed(unit="m/s");
  ModelicaMaritime.Interfaces.RealOutput headingError(unit="rad");
  ModelicaMaritime.Interfaces.RealOutput depthError(unit="m");
  ModelicaMaritime.Interfaces.RealOutput speedError(unit="m/s");
equation
  headingError = atan2(
    sin(commandedHeading - heading),
    cos(commandedHeading - heading));
  depthError = commandedDepth - depth;
  speedError = commandedSpeed - speed;
end CommandErrors;
