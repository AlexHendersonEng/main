within ModelicaAutomotive.Suspension;
block BumpStop "Progressive compression and rebound travel stops"
  parameter ModelicaAutomotive.Types.Length compressionTravel = 0.08;
  parameter ModelicaAutomotive.Types.Length reboundTravel = 0.08;
  parameter Real compressionStiffness(unit="N/m") = 200000;
  parameter Real reboundStiffness(unit="N/m") = 100000;
  ModelicaAutomotive.Interfaces.RealInput compression(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput forceOnBody(unit="N");
equation
  assert(compressionTravel >= 0, "compressionTravel must not be negative");
  assert(reboundTravel >= 0, "reboundTravel must not be negative");
  assert(compressionStiffness >= 0, "compressionStiffness must not be negative");
  assert(reboundStiffness >= 0, "reboundStiffness must not be negative");
  forceOnBody =
    if compression > compressionTravel then
      compressionStiffness * (compression - compressionTravel)
    elseif compression < -reboundTravel then
      reboundStiffness * (compression + reboundTravel)
    else 0;
end BumpStop;
