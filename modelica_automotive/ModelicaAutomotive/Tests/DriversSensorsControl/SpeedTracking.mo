within ModelicaAutomotive.Tests.DriversSensorsControl;
model SpeedTracking "Closed-loop speed tracking with propulsion and braking"
  ModelicaAutomotive.Drivers.SpeedController controller(
    proportionalGain=0.22,
    integralGain=0.06,
    antiWindupGain=3);
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=1400,
    initialSpeed=0,
    dragArea=0.65,
    rollingResistanceCoefficient=0.01,
    resistanceRegularization=0.05);
  output Real targetSpeed;
  output Real speed;
  output Real propulsionCommand;
  output Real brakeCommand;
  output Real integralState;
  output Real speedError;
equation
  targetSpeed = if time < 10 then 15 else 8;
  controller.targetSpeed = targetSpeed;
  controller.measuredSpeed = body.speed;
  controller.enable = 1;
  body.tireForce =
    5000 * controller.propulsionCommand
    - 7000 * controller.brakeCommand;
  body.roadGrade = 0;
  body.windSpeed = 0;
  speed = body.speed;
  propulsionCommand = controller.propulsionCommand;
  brakeCommand = controller.brakeCommand;
  integralState = controller.integralState;
  speedError = controller.speedError;
end SpeedTracking;
