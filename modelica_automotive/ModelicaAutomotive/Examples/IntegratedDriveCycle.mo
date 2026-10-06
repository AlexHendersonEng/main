within ModelicaAutomotive.Examples;
model IntegratedDriveCycle "Closed-loop speed cycle with regeneration and energy accounting"
  parameter Real wheelRadius(unit="m") = 0.32;
  ModelicaAutomotive.Drivers.SpeedController driver(
    proportionalGain=0.18,
    integralGain=0.05,
    antiWindupGain=3);
  ModelicaAutomotive.Control.BrakeBlending blending(
    maximumRegenerativeFraction=0.65);
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=1500,
    initialSpeed=0,
    dragArea=0.7,
    rollingResistanceCoefficient=0.012,
    resistanceRegularization=0.05);
  ModelicaAutomotive.Powertrain.FirstOrderMachine machine(
    parameters(
      maximumDriveTorque=300,
      maximumRegenerativeTorque=180,
      baseSpeed=400,
      maximumSpeed=700,
      responseTime=0.15,
      driveEfficiency=0.92,
      regenerativeEfficiency=0.8));
  ModelicaAutomotive.Powertrain.FixedRatio transmission(
    ratio=9,
    efficiency=0.96);
  ModelicaAutomotive.Powertrain.EnergyStorage storage(
    capacity=2e8,
    initialStateOfCharge=0.75);
  ModelicaAutomotive.Scenarios.ScenarioMetrics metrics;
  output Real targetSpeed(unit="m/s");
  output Real speed(unit="m/s");
  output Real position(unit="m");
  output Real propulsionCommand;
  output Real brakeCommand;
  output Real regenerativeCommand;
  output Real frictionBrakeCommand;
  output Real stateOfCharge;
  output Real sourcePower(unit="W");
  output Real distanceTravelled(unit="m");
equation
  targetSpeed =
    if time < 5 then 10
    elseif time < 12 then 18
    elseif time < 18 then 8
    else 0;
  driver.targetSpeed = targetSpeed;
  driver.measuredSpeed = body.speed;
  driver.enable = 1;
  blending.brakeCommand = driver.brakeCommand;
  blending.regenerativeAvailability = 0.65;
  transmission.outputAngularVelocity = body.speed / wheelRadius;
  machine.command = driver.propulsionCommand + blending.regenerativeCommand;
  machine.angularVelocity = transmission.inputAngularVelocity;
  transmission.inputTorque = machine.torque;
  storage.sourcePower = machine.sourcePower;
  body.tireForce = transmission.outputTorque / wheelRadius
    - 7000 * blending.frictionBrakeCommand;
  body.roadGrade = 0;
  body.windSpeed = 0;
  metrics.speed = body.speed;
  metrics.lateralError = 0;
  metrics.yawRate = 0;
  metrics.controlEffort = driver.controlEffort;
  speed = body.speed;
  position = body.position;
  propulsionCommand = driver.propulsionCommand;
  brakeCommand = driver.brakeCommand;
  regenerativeCommand = blending.regenerativeCommand;
  frictionBrakeCommand = blending.frictionBrakeCommand;
  stateOfCharge = storage.stateOfCharge;
  sourcePower = machine.sourcePower;
  distanceTravelled = metrics.distanceTravelled;
  annotation (
    experiment(StartTime=0, StopTime=24, Tolerance=1e-8, Interval=0.02),
    Documentation(info="<html><p>A closed-loop urban-style speed cycle combines driver control, regenerative brake blending, machine dynamics, gearing, road loads, and storage energy.</p></html>"));
end IntegratedDriveCycle;
