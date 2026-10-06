within ModelicaMaritime.Tests.GNC;
model ControlValidation "Validate saturation, anti-windup, heading wrap, and allocation"
  ModelicaMaritime.Control.LimitedPID pid(
    k=2,
    Ti=0.5,
    outputMaximum=1,
    outputMinimum=-1,
    antiWindup=5);
  ModelicaMaritime.Control.HeadingController heading(
    k=2,
    Ti=1,
    outputMaximum=0.5,
    outputMinimum=-0.5);
  ModelicaMaritime.Control.PlanarAllocator allocator(
    differentialYawGain=0.5,
    rudderYawGain=1.5);
  output Real command;
  output Real controlError;
  output Real headingCommand;
  output Real headingError;
  output Real portCommand;
  output Real starboardCommand;
  output Real rudderCommand;
  output Real plantMeasurement;
protected
  Real measurementState(start=0, fixed=true);
  Real probe;
equation
  probe = 1e-8 * time;
  pid.setpoint = if time < 2 then 10 else 0;
  pid.measurement = measurementState;
  der(measurementState) = (pid.command - measurementState) / 0.5;
  heading.commandedHeading = 0.1;
  heading.heading = 6.2;
  allocator.surgeCommand = 0.8;
  allocator.yawCommand = 0.6;
  command = pid.command;
  controlError = pid.error;
  headingCommand = heading.command;
  headingError = heading.headingError;
  portCommand = allocator.portThrusterCommand + probe;
  starboardCommand = allocator.starboardThrusterCommand + 2 * probe;
  rudderCommand = allocator.rudderCommand + 3 * probe;
  plantMeasurement = measurementState;
end ControlValidation;
