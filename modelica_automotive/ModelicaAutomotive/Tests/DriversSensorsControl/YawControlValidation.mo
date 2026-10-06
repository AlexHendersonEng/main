within ModelicaAutomotive.Tests.DriversSensorsControl;
model YawControlValidation "Direct yaw moment rejects a constant disturbance"
  parameter Real yawInertia(unit="kg.m2") = 2500;
  parameter Real damping(unit="N.m.s/rad") = 1000;
  parameter Real disturbanceMoment(unit="N.m") = 1500;
  ModelicaAutomotive.Control.YawStabilityControl controller(
    proportionalGain=12000,
    deadband=0.005,
    maximumYawMoment=3000);
  output Real uncontrolledYawRate(start=0, fixed=true, unit="rad/s");
  output Real controlledYawRate(start=0, fixed=true, unit="rad/s");
  output Real yawMomentCommand(unit="N.m");
  output Real controllerActive;
equation
  controller.desiredYawRate = 0;
  controller.measuredYawRate = controlledYawRate;
  der(uncontrolledYawRate) =
    (disturbanceMoment - damping * uncontrolledYawRate) / yawInertia;
  der(controlledYawRate) =
    (disturbanceMoment + controller.yawMomentCommand
      - damping * controlledYawRate) / yawInertia;
  yawMomentCommand = controller.yawMomentCommand;
  controllerActive = controller.active;
end YawControlValidation;
