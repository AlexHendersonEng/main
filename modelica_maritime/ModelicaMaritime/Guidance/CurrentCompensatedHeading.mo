within ModelicaMaritime.Guidance;
block CurrentCompensatedHeading "Heading required to achieve desired ground track in current"
  ModelicaMaritime.Interfaces.RealInput desiredGroundTrack(unit="rad");
  ModelicaMaritime.Interfaces.RealInput desiredGroundSpeed(unit="m/s");
  ModelicaMaritime.Interfaces.Vector3Input currentVelocityNED(each unit="m/s");
  ModelicaMaritime.Interfaces.RealOutput commandedHeading(unit="rad");
  ModelicaMaritime.Interfaces.RealOutput requiredWaterSpeed(unit="m/s");
protected
  Real requiredNorth(unit="m/s");
  Real requiredEast(unit="m/s");
equation
  assert(desiredGroundSpeed >= 0, "Desired ground speed must be non-negative");
  requiredNorth = desiredGroundSpeed * cos(desiredGroundTrack) - currentVelocityNED[1];
  requiredEast = desiredGroundSpeed * sin(desiredGroundTrack) - currentVelocityNED[2];
  requiredWaterSpeed = sqrt(requiredNorth * requiredNorth + requiredEast * requiredEast);
  commandedHeading = ModelicaMaritime.Mathematics.wrapHeading(
    atan2(requiredEast, requiredNorth));
end CurrentCompensatedHeading;
