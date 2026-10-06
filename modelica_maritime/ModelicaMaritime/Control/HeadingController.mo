within ModelicaMaritime.Control;
block HeadingController "Wrapped-heading limited PI/PID controller"
  parameter Real k = 1;
  parameter Real Ti(unit="s") = 1;
  parameter Real Td(unit="s") = 0.1;
  parameter Boolean derivativeEnabled = false;
  parameter Real outputMaximum = 1;
  parameter Real outputMinimum = -outputMaximum;
  ModelicaMaritime.Interfaces.RealInput commandedHeading(unit="rad");
  ModelicaMaritime.Interfaces.RealInput heading(unit="rad");
  ModelicaMaritime.Interfaces.RealOutput command;
  ModelicaMaritime.Interfaces.RealOutput headingError(unit="rad");
protected
  ModelicaMaritime.Control.LimitedPID controller(
    k=k,
    Ti=Ti,
    Td=Td,
    derivativeEnabled=derivativeEnabled,
    outputMaximum=outputMaximum,
    outputMinimum=outputMinimum);
equation
  headingError = atan2(
    sin(commandedHeading - heading),
    cos(commandedHeading - heading));
  controller.setpoint = 0;
  controller.measurement = -headingError;
  command = controller.command;
end HeadingController;
