within ModelicaMaritime.Actuators;
block BallastTank "Rate-limited ballast-water mass state"
  parameter ModelicaMaritime.Types.Mass capacity = 1;
  parameter ModelicaMaritime.Types.Mass initialMass = 0;
  parameter Real maximumFillRate(unit="kg/s", min=0) = 1;
  parameter Real maximumEmptyRate(unit="kg/s", min=0) = 1;
  parameter Real gravity(unit="m/s2") =
    ModelicaMaritime.Constants.standardGravity;
  ModelicaMaritime.Interfaces.RealInput pumpCommand
    "Normalized command: positive fills and negative empties";
  input Boolean failed;
  ModelicaMaritime.Interfaces.RealOutput ballastMass(unit="kg");
  ModelicaMaritime.Interfaces.RealOutput massRate(unit="kg/s");
  ModelicaMaritime.Interfaces.Vector6Output generalizedLoadBody;
protected
  Real massState(start=initialMass, fixed=true, unit="kg");
  Real limitedCommand;
  Real requestedRate(unit="kg/s");
equation
  assert(capacity > 0, "Ballast capacity must be positive");
  assert(initialMass >= 0 and initialMass <= capacity, "Initial ballast mass is invalid");
  limitedCommand = min(max(pumpCommand, -1), 1);
  requestedRate = if failed then 0
    else if limitedCommand >= 0 then limitedCommand * maximumFillRate
    else limitedCommand * maximumEmptyRate;
  massRate = if massState >= capacity and requestedRate > 0 then 0
    else if massState <= 0 and requestedRate < 0 then 0
    else requestedRate;
  der(massState) = massRate;
  ballastMass = min(max(massState, 0), capacity);
  generalizedLoadBody = {0, 0, ballastMass * gravity, 0, 0, 0};
end BallastTank;
