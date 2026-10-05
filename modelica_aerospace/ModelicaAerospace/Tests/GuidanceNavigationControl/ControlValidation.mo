within ModelicaAerospace.Tests.GuidanceNavigationControl;
model ControlValidation "Exercise MSL control wrappers and a closed loop"
  ModelicaAerospace.Control.LimitedPID pid(
    controllerType=Modelica.Blocks.Types.SimpleController.PI,
    k=2,
    Ti=0.5,
    outputMinimum=-1,
    outputMaximum=1,
    antiWindup=0.2);
  ModelicaAerospace.Control.GainSchedule schedule(
    schedulingPoints={0, 1, 2},
    proportionalGain={1, 2, 4},
    integralGain={0.1, 0.2, 0.4},
    derivativeGain={0, 0.5, 1});
  ModelicaAerospace.Control.ModeSelector selector;
  ModelicaAerospace.Control.CommandLimiter limiter(
    minimum=-1,
    maximum=1,
    risingRate=0.5,
    fallingRate=-1,
    initialOutput=0);
  output Real response(start=0, fixed=true);
  output Real pidCommand;
  output Real controlError;
  output Real scheduledGains[3];
  output Real selectedCommand;
  output Real limitedCommand;
protected
  Real setpoint;
equation
  setpoint = if time < 0.5 then 0 else if time < 3 then 2 else 0;
  pid.setpoint = setpoint;
  pid.measurement = response;
  pidCommand = pid.command;
  controlError = pid.error;
  der(response) = (-response + pidCommand) / 0.5;

  schedule.schedulingVariable =
    if time < 1 then -1
    else if time < 2 then 0.5
    else if time < 3 then 1
    else 3;
  scheduledGains = schedule.gains;

  selector.primaryCommand = 2;
  selector.alternateCommand = -2;
  selector.usePrimary = time >= 1.5;
  selectedCommand = selector.command;

  limiter.rawCommand =
    if time < 0.5 then 0
    else if time < 3.5 then 2
    else -2;
  limitedCommand = limiter.limitedCommand;
end ControlValidation;
