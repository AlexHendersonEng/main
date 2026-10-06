within ModelicaMaritime.Propulsion;
block FixedThruster "Fixed-direction commanded propulsor load"
  parameter ModelicaMaritime.Types.Force maximumForwardThrust = 1;
  parameter ModelicaMaritime.Types.Force maximumReverseThrust = maximumForwardThrust;
  parameter ModelicaMaritime.Types.Vector3 directionBody = {1, 0, 0};
  parameter ModelicaMaritime.Types.Vector3 applicationPointBody = {0, 0, 0};
  ModelicaMaritime.Interfaces.RealInput command;
  input Boolean enabled;
  input Boolean failed;
  ModelicaMaritime.Interfaces.RealOutput thrust(unit="N");
  ModelicaMaritime.Interfaces.Vector6Output generalizedLoadBody;
protected
  Real limitedCommand;
equation
  assert(maximumForwardThrust >= 0, "Maximum forward thrust must be non-negative");
  assert(maximumReverseThrust >= 0, "Maximum reverse thrust must be non-negative");
  limitedCommand = min(max(command, -1), 1);
  thrust = if enabled and not failed then
    if limitedCommand >= 0 then limitedCommand * maximumForwardThrust
    else limitedCommand * maximumReverseThrust
    else 0;
  generalizedLoadBody = ModelicaMaritime.Propulsion.propulsorLoad(
    thrust,
    directionBody,
    applicationPointBody);
end FixedThruster;
