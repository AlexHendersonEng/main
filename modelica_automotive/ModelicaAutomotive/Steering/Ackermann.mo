within ModelicaAutomotive.Steering;
block Ackermann "Convert a central steer angle to four road-wheel angles"
  parameter ModelicaAutomotive.Types.VehicleGeometry geometry;
  ModelicaAutomotive.Interfaces.RealInput steeringAngle(unit="rad");
  ModelicaAutomotive.Interfaces.CornerOutput wheelAngles(each unit="rad");
protected
  Real tangent;
equation
  assert(geometry.wheelbase > 0, "wheelbase must be positive");
  assert(geometry.frontTrack > 0, "frontTrack must be positive");
  tangent = tan(steeringAngle);
  wheelAngles[ModelicaAutomotive.Constants.frontLeft] = atan(
    geometry.wheelbase * tangent
    / (geometry.wheelbase - 0.5 * geometry.frontTrack * tangent));
  wheelAngles[ModelicaAutomotive.Constants.frontRight] = atan(
    geometry.wheelbase * tangent
    / (geometry.wheelbase + 0.5 * geometry.frontTrack * tangent));
  wheelAngles[ModelicaAutomotive.Constants.rearLeft] = 0;
  wheelAngles[ModelicaAutomotive.Constants.rearRight] = 0;
end Ackermann;
