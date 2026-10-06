within ModelicaAutomotive.Examples;
model GradeShiftAcceleration "Accelerate uphill through a two-speed transmission"
  parameter Real wheelRadius(unit="m")=0.32;
  ModelicaAutomotive.VehicleDynamics.Longitudinal.Body body(
    mass=1500,
    initialSpeed=2,
    dragArea=0.7,
    rollingResistanceCoefficient=0.012,
    resistanceRegularization=0.05);
  ModelicaAutomotive.Powertrain.FirstOrderMachine machine(
    parameters(
      maximumDriveTorque=280,
      maximumRegenerativeTorque=120,
      baseSpeed=350,
      maximumSpeed=650,
      responseTime=0.2,
      driveEfficiency=0.92,
      regenerativeEfficiency=0.75));
  ModelicaAutomotive.Powertrain.SelectableTransmission transmission(
    ratios={10, 6},
    efficiency=0.96);
  ModelicaAutomotive.Powertrain.EnergyStorage storage(
    capacity=2e8,
    initialStateOfCharge=0.8);
  output Integer gear;
  output Real position(unit="m");
  output Real speed(unit="m/s");
  output Real acceleration(unit="m/s2");
  output Real machineTorque(unit="N.m");
  output Real driveForce(unit="N");
  output Real stateOfCharge;
equation
  gear = if time < 4 then 1 else 2;
  transmission.gearCommand = gear;
  transmission.outputAngularVelocity = body.speed / wheelRadius;
  machine.command = 1;
  machine.angularVelocity = transmission.inputAngularVelocity;
  transmission.inputTorque = machine.torque;
  storage.sourcePower = machine.sourcePower;
  driveForce = transmission.outputTorque / wheelRadius;
  body.tireForce = driveForce;
  body.roadGrade = 0.05;
  body.windSpeed = 0;
  position = body.position;
  speed = body.speed;
  acceleration = body.acceleration;
  machineTorque = machine.torque;
  stateOfCharge = storage.stateOfCharge;
  annotation (
    experiment(StartTime=0, StopTime=12, Tolerance=1e-8, Interval=0.01),
    Documentation(info="<html>
<p>A first-order electric machine accelerates a passenger vehicle up a
constant grade and shifts from a 10:1 to a 6:1 ratio after four seconds.</p>
</html>"));
end GradeShiftAcceleration;
