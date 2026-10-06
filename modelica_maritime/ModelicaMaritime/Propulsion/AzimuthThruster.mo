within ModelicaMaritime.Propulsion;
block AzimuthThruster "Horizontal azimuthing commanded propulsor load"
  parameter ModelicaMaritime.Types.Force maximumForwardThrust = 1;
  parameter ModelicaMaritime.Types.Force maximumReverseThrust = maximumForwardThrust;
  parameter ModelicaMaritime.Types.Vector3 applicationPointBody = {0, 0, 0};
  ModelicaMaritime.Interfaces.RealInput command;
  ModelicaMaritime.Interfaces.RealInput azimuth(unit="rad");
  input Boolean enabled;
  input Boolean failed;
  ModelicaMaritime.Interfaces.RealOutput thrust(unit="N");
  ModelicaMaritime.Interfaces.Vector6Output generalizedLoadBody;
protected
  Real limitedCommand;
  Real directionBody[3];
equation
  assert(maximumForwardThrust >= 0, "Maximum forward thrust must be non-negative");
  assert(maximumReverseThrust >= 0, "Maximum reverse thrust must be non-negative");
  limitedCommand = min(max(command, -1), 1);
  thrust = if enabled and not failed then
    if limitedCommand >= 0 then limitedCommand * maximumForwardThrust
    else limitedCommand * maximumReverseThrust
    else 0;
  directionBody = {cos(azimuth), sin(azimuth), 0};
  generalizedLoadBody = ModelicaMaritime.Propulsion.propulsorLoad(
    thrust,
    directionBody,
    applicationPointBody);
end AzimuthThruster;
