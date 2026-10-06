within ModelicaMaritime.Control;
block PlanarAllocator "Allocate normalized surge/yaw demand to twin thrusters and rudder"
  parameter Real differentialYawGain = 1;
  parameter Real rudderYawGain = 1;
  ModelicaMaritime.Interfaces.RealInput surgeCommand;
  ModelicaMaritime.Interfaces.RealInput yawCommand;
  ModelicaMaritime.Interfaces.RealOutput portThrusterCommand;
  ModelicaMaritime.Interfaces.RealOutput starboardThrusterCommand;
  ModelicaMaritime.Interfaces.RealOutput rudderCommand;
protected
  Real limitedSurge;
  Real limitedYaw;
equation
  assert(differentialYawGain >= 0, "Differential yaw gain must be non-negative");
  assert(rudderYawGain >= 0, "Rudder yaw gain must be non-negative");
  limitedSurge = min(max(surgeCommand, -1), 1);
  limitedYaw = min(max(yawCommand, -1), 1);
  portThrusterCommand = min(
    max(limitedSurge + differentialYawGain * limitedYaw, -1),
    1);
  starboardThrusterCommand = min(
    max(limitedSurge - differentialYawGain * limitedYaw, -1),
    1);
  rudderCommand = min(max(rudderYawGain * limitedYaw, -1), 1);
end PlanarAllocator;
