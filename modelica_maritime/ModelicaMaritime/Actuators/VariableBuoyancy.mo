within ModelicaMaritime.Actuators;
block VariableBuoyancy "Rate-limited variable displaced-volume abstraction"
  parameter Real minimumVolume(unit="m3", min=0) = 0;
  parameter Real maximumVolume(unit="m3") = 1;
  parameter Real initialVolume(unit="m3") = minimumVolume;
  parameter Real maximumVolumeRate(unit="m3/s", min=0) = 0.01;
  parameter ModelicaMaritime.Types.Density waterDensity =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter Real gravity(unit="m/s2") =
    ModelicaMaritime.Constants.standardGravity;
  ModelicaMaritime.Interfaces.RealInput command
    "Normalized command: positive increases displaced volume";
  input Boolean failed;
  ModelicaMaritime.Interfaces.RealOutput displacedVolume(unit="m3");
  ModelicaMaritime.Interfaces.RealOutput volumeRate(unit="m3/s");
  ModelicaMaritime.Interfaces.Vector6Output generalizedLoadBody;
protected
  Real volumeState(start=initialVolume, fixed=true, unit="m3");
  Real requestedRate(unit="m3/s");
equation
  assert(maximumVolume > minimumVolume, "Variable-buoyancy volume bounds are invalid");
  assert(
    initialVolume >= minimumVolume and initialVolume <= maximumVolume,
    "Initial variable-buoyancy volume is invalid");
  requestedRate = if failed then 0
    else min(max(command, -1), 1) * maximumVolumeRate;
  volumeRate = if volumeState >= maximumVolume and requestedRate > 0 then 0
    else if volumeState <= minimumVolume and requestedRate < 0 then 0
    else requestedRate;
  der(volumeState) = volumeRate;
  displacedVolume = min(max(volumeState, minimumVolume), maximumVolume);
  generalizedLoadBody = {
    0,
    0,
    -waterDensity * gravity * (displacedVolume - initialVolume),
    0,
    0,
    0};
end VariableBuoyancy;
