within ModelicaAerospace.Control;
block LimitedPID "MSL limited PID controller with anti-windup"
  parameter Modelica.Blocks.Types.SimpleController controllerType =
    Modelica.Blocks.Types.SimpleController.PID;
  parameter Real k = 1;
  parameter Real Ti(unit="s") = 0.5;
  parameter Real Td(unit="s") = 0.1;
  parameter Real outputMaximum = 1;
  parameter Real outputMinimum = -outputMaximum;
  parameter Real antiWindup = 0.9;
  parameter Real derivativeFilter = 10;
  parameter Real initialOutput = 0;
  ModelicaAerospace.Interfaces.RealInput setpoint;
  ModelicaAerospace.Interfaces.RealInput measurement;
  ModelicaAerospace.Interfaces.RealOutput command;
  ModelicaAerospace.Interfaces.RealOutput error;
protected
  Modelica.Blocks.Continuous.LimPID controller(
    controllerType=controllerType,
    k=k,
    Ti=Ti,
    Td=Td,
    yMax=outputMaximum,
    yMin=outputMinimum,
    Ni=antiWindup,
    Nd=derivativeFilter,
    initType=Modelica.Blocks.Types.Init.InitialOutput,
    y_start=initialOutput);
equation
  controller.u_s = setpoint;
  controller.u_m = measurement;
  command = controller.y;
  error = controller.controlError;
end LimitedPID;
