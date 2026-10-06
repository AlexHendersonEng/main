within ModelicaAutomotive.Examples;
model StabilityControlledTurn "Direct-yaw-moment stabilization during a disturbed turn"
  parameter Real speed(unit="m/s") = 15;
  parameter Real yawInertia(unit="kg.m2") = 2500;
  parameter Real yawDamping(unit="N.m.s/rad") = 1200;
  ModelicaAutomotive.Control.YawStabilityControl controller(
    proportionalGain=14000,
    deadband=0.005,
    maximumYawMoment=3500);
  ModelicaAutomotive.Scenarios.ScenarioMetrics metrics;
  output Real position[2](each unit="m");
  output Real yaw(unit="rad");
  output Real yawRate(start=0, fixed=true, unit="rad/s");
  output Real desiredYawRate(unit="rad/s");
  output Real yawMomentCommand(unit="N.m");
  output Real disturbanceMoment(unit="N.m");
protected
  Real positionX(start=0, fixed=true, unit="m");
  Real positionY(start=0, fixed=true, unit="m");
  Real yawState(start=0, fixed=true, unit="rad");
equation
  desiredYawRate = if time >= 1 and time < 5 then 0.2 else 0;
  disturbanceMoment =
    if time >= 2 and time < 3 then 1800 else 0;
  controller.desiredYawRate = desiredYawRate;
  controller.measuredYawRate = yawRate;
  der(yawRate) =
    (controller.yawMomentCommand + disturbanceMoment
      - yawDamping * yawRate) / yawInertia;
  der(yawState) = yawRate;
  der(positionX) = speed * cos(yawState);
  der(positionY) = speed * sin(yawState);
  metrics.speed = speed;
  metrics.lateralError = 0;
  metrics.yawRate = yawRate;
  metrics.controlEffort = controller.yawMomentCommand
    / controller.maximumYawMoment;
  position = {positionX, positionY};
  yaw = yawState;
  yawMomentCommand = controller.yawMomentCommand;
  annotation (
    experiment(StartTime=0, StopTime=8, Tolerance=1e-8, Interval=0.01),
    Documentation(info="<html><p>A direct-yaw-moment controller tracks a bounded turn command while rejecting a one-second external yaw disturbance.</p></html>"));
end StabilityControlledTurn;
