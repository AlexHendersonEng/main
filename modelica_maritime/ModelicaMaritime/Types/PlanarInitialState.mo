within ModelicaMaritime.Types;
record PlanarInitialState "Initial surge, sway, and yaw state"
  ModelicaMaritime.Types.Length positionNED[2] = {0, 0}
    "Initial north and east position";
  ModelicaMaritime.Types.Angle heading = 0 "Initial heading clockwise from north";
  ModelicaMaritime.Types.Velocity velocityBody[2] = {0, 0}
    "Initial surge and sway velocity";
  ModelicaMaritime.Types.AngularVelocity yawRate = 0 "Initial body yaw rate";
end PlanarInitialState;
