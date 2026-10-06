within ModelicaAutomotive.Examples;
model RegenerativeBraking "Decelerate while returning energy to storage"
  parameter Real wheelRadius(unit="m")=0.32;
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=1500,
    initialSpeed=25,
    dragArea=0.7,
    rollingResistanceCoefficient=0.012,
    resistanceRegularization=0.05);
  ModelicaAutomotive.Powertrain.FirstOrderMachine machine(
    parameters(
      maximumDriveTorque=280,
      maximumRegenerativeTorque=180,
      baseSpeed=350,
      maximumSpeed=650,
      responseTime=0.15,
      driveEfficiency=0.92,
      regenerativeEfficiency=0.75));
  ModelicaAutomotive.Powertrain.FixedRatio transmission(
    ratio=6,
    efficiency=0.96);
  ModelicaAutomotive.Powertrain.EnergyStorage storage(
    capacity=2e8,
    initialStateOfCharge=0.5);
  output Real speed(unit="m/s");
  output Real acceleration(unit="m/s2");
  output Real machineTorque(unit="N.m");
  output Real sourcePower(unit="W");
  output Real stateOfCharge;
equation
  transmission.outputAngularVelocity = body.speed / wheelRadius;
  machine.command = -0.7;
  machine.angularVelocity = transmission.inputAngularVelocity;
  transmission.inputTorque = machine.torque;
  storage.sourcePower = machine.sourcePower;
  body.tireForce = transmission.outputTorque / wheelRadius;
  body.roadGrade = 0;
  body.windSpeed = 0;
  speed = body.speed;
  acceleration = body.acceleration;
  machineTorque = machine.torque;
  sourcePower = machine.sourcePower;
  stateOfCharge = storage.stateOfCharge;
  annotation (
    experiment(StartTime=0, StopTime=8, Tolerance=1e-8, Interval=0.01),
    Documentation(info="<html>
<p>A regenerative torque command decelerates the vehicle while increasing
the energy-storage state of charge.</p>
</html>"));
end RegenerativeBraking;
