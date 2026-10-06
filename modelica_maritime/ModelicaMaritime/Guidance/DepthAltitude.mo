within ModelicaMaritime.Guidance;
block DepthAltitude "Select positive-down depth or positive-up altitude guidance"
  parameter Boolean altitudeMode = false;
  ModelicaMaritime.Interfaces.RealInput requestedDepth(unit="m");
  ModelicaMaritime.Interfaces.RealInput requestedAltitude(unit="m");
  ModelicaMaritime.Interfaces.RealInput seafloorDepth(unit="m");
  ModelicaMaritime.Interfaces.RealOutput commandedDepth(unit="m");
equation
  commandedDepth = if altitudeMode then
    seafloorDepth - requestedAltitude else requestedDepth;
  assert(commandedDepth >= 0, "Guidance commanded a depth above the water surface");
end DepthAltitude;
