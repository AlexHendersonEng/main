within ModelicaAutomotive.Suspension;
block HeaveRollPitchBody "Sprung-body heave, roll, and pitch dynamics"
  parameter ModelicaAutomotive.Types.Mass mass = 1500;
  parameter ModelicaAutomotive.Types.Inertia rollInertia = 650;
  parameter ModelicaAutomotive.Types.Inertia pitchInertia = 2200;
  parameter ModelicaAutomotive.Types.VehicleGeometry geometry;
  parameter ModelicaAutomotive.Types.Acceleration gravity =
    ModelicaAutomotive.Constants.standardGravity;
  parameter ModelicaAutomotive.Types.Length initialHeave = 0;
  parameter ModelicaAutomotive.Types.Angle initialRoll = 0;
  parameter ModelicaAutomotive.Types.Angle initialPitch = 0;
  ModelicaAutomotive.Interfaces.CornerInput verticalForce(each unit="N")
    "Positive-up force on the sprung body";
  ModelicaAutomotive.Interfaces.RealInput externalRollMoment(unit="N.m");
  ModelicaAutomotive.Interfaces.RealInput externalPitchMoment(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput heave(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput roll(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput pitch(unit="rad")
    "Positive nose-down";
  ModelicaAutomotive.Interfaces.RealOutput heaveVelocity(unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput rollRate(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput pitchRate(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput heaveAcceleration(unit="m/s2");
  ModelicaAutomotive.Interfaces.RealOutput rollAcceleration(unit="rad/s2");
  ModelicaAutomotive.Interfaces.RealOutput pitchAcceleration(unit="rad/s2");
protected
  parameter Real rearDistance(unit="m") =
    geometry.wheelbase - geometry.centerOfMassToFrontAxle;
  Real heaveState(start=initialHeave, fixed=true, unit="m");
  Real rollState(start=initialRoll, fixed=true, unit="rad");
  Real pitchState(start=initialPitch, fixed=true, unit="rad");
  Real heaveVelocityState(start=0, fixed=true, unit="m/s");
  Real rollRateState(start=0, fixed=true, unit="rad/s");
  Real pitchRateState(start=0, fixed=true, unit="rad/s");
  Real positionX[4](each unit="m");
  Real positionY[4](each unit="m");
equation
  assert(mass > 0, "mass must be positive");
  assert(rollInertia > 0, "rollInertia must be positive");
  assert(pitchInertia > 0, "pitchInertia must be positive");
  assert(geometry.centerOfMassToFrontAxle > 0 and rearDistance > 0,
    "Center of mass must lie between the axles");
  positionX = {
    geometry.centerOfMassToFrontAxle,
    geometry.centerOfMassToFrontAxle,
    -rearDistance,
    -rearDistance};
  positionY = {
    geometry.frontTrack / 2,
    -geometry.frontTrack / 2,
    geometry.rearTrack / 2,
    -geometry.rearTrack / 2};
  heaveAcceleration = (sum(verticalForce) - mass * gravity) / mass;
  rollAcceleration =
    (sum(positionY .* verticalForce) + externalRollMoment) / rollInertia;
  pitchAcceleration =
    (-sum(positionX .* verticalForce) + externalPitchMoment) / pitchInertia;
  der(heaveState) = heaveVelocityState;
  der(rollState) = rollRateState;
  der(pitchState) = pitchRateState;
  der(heaveVelocityState) = heaveAcceleration;
  der(rollRateState) = rollAcceleration;
  der(pitchRateState) = pitchAcceleration;
  heave = heaveState;
  roll = rollState;
  pitch = pitchState;
  heaveVelocity = heaveVelocityState;
  rollRate = rollRateState;
  pitchRate = pitchRateState;
end HeaveRollPitchBody;
