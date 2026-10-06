within ModelicaAutomotive.Suspension;
block FourCornerSuspension "Reduced sprung body supported by four corner suspensions"
  parameter ModelicaAutomotive.Types.Mass mass = 1500;
  parameter ModelicaAutomotive.Types.Inertia rollInertia = 650;
  parameter ModelicaAutomotive.Types.Inertia pitchInertia = 2200;
  parameter ModelicaAutomotive.Types.VehicleGeometry geometry;
  parameter Real cornerStiffness(unit="N/m") = 30000;
  parameter Real cornerDamping(unit="N.s/m") = 3000;
  parameter Real frontAntiRollStiffness(unit="N/m") = 0;
  parameter Real rearAntiRollStiffness(unit="N/m") = 0;
  parameter ModelicaAutomotive.Types.Length bumpTravel = 0.08;
  parameter ModelicaAutomotive.Types.Length reboundTravel = 0.08;
  parameter Real bumpStiffness(unit="N/m") = 200000;
  parameter Real reboundStiffness(unit="N/m") = 100000;
  parameter ModelicaAutomotive.Types.Length initialHeave = 0;
  parameter ModelicaAutomotive.Types.Angle initialRoll = 0;
  parameter ModelicaAutomotive.Types.Angle initialPitch = 0;
  ModelicaAutomotive.Interfaces.CornerInput roadHeight(each unit="m");
  ModelicaAutomotive.Interfaces.CornerInput roadVelocity(each unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput heave(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput roll(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput pitch(unit="rad");
  ModelicaAutomotive.Interfaces.CornerOutput suspensionForce(each unit="N");
  ModelicaAutomotive.Interfaces.CornerOutput compression(each unit="m");
protected
  parameter Real rearDistance(unit="m") =
    geometry.wheelbase - geometry.centerOfMassToFrontAxle;
  parameter Real positionX[4](each unit="m") = {
    geometry.centerOfMassToFrontAxle,
    geometry.centerOfMassToFrontAxle,
    -rearDistance,
    -rearDistance};
  parameter Real positionY[4](each unit="m") = {
    geometry.frontTrack / 2,
    -geometry.frontTrack / 2,
    geometry.rearTrack / 2,
    -geometry.rearTrack / 2};
  parameter Real preload[4](each unit="N") = {
    mass * ModelicaAutomotive.Constants.standardGravity * rearDistance
      / (2 * geometry.wheelbase),
    mass * ModelicaAutomotive.Constants.standardGravity * rearDistance
      / (2 * geometry.wheelbase),
    mass * ModelicaAutomotive.Constants.standardGravity
      * geometry.centerOfMassToFrontAxle / (2 * geometry.wheelbase),
    mass * ModelicaAutomotive.Constants.standardGravity
      * geometry.centerOfMassToFrontAxle / (2 * geometry.wheelbase)};
  ModelicaAutomotive.Suspension.HeaveRollPitchBody body(
    mass=mass,
    rollInertia=rollInertia,
    pitchInertia=pitchInertia,
    geometry=geometry,
    initialHeave=initialHeave,
    initialRoll=initialRoll,
    initialPitch=initialPitch);
  Real bodyCornerHeight[4](each unit="m");
  Real bodyCornerVelocity[4](each unit="m/s");
  Real compressionRate[4](each unit="m/s");
  Real stopForce[4](each unit="N");
  Real antiRollForce[4](each unit="N");
equation
  assert(cornerStiffness >= 0, "cornerStiffness must not be negative");
  assert(cornerDamping >= 0, "cornerDamping must not be negative");
  assert(frontAntiRollStiffness >= 0,
    "frontAntiRollStiffness must not be negative");
  assert(rearAntiRollStiffness >= 0,
    "rearAntiRollStiffness must not be negative");
  for corner in 1:4 loop
    bodyCornerHeight[corner] =
      body.heave + body.roll * positionY[corner] - body.pitch * positionX[corner];
    bodyCornerVelocity[corner] =
      body.heaveVelocity + body.rollRate * positionY[corner]
      - body.pitchRate * positionX[corner];
    compression[corner] = roadHeight[corner] - bodyCornerHeight[corner];
    compressionRate[corner] = roadVelocity[corner] - bodyCornerVelocity[corner];
    stopForce[corner] =
      if compression[corner] > bumpTravel then
        bumpStiffness * (compression[corner] - bumpTravel)
      elseif compression[corner] < -reboundTravel then
        reboundStiffness * (compression[corner] + reboundTravel)
      else 0;
  end for;
  antiRollForce = {
    frontAntiRollStiffness * (compression[1] - compression[2]),
    -frontAntiRollStiffness * (compression[1] - compression[2]),
    rearAntiRollStiffness * (compression[3] - compression[4]),
    -rearAntiRollStiffness * (compression[3] - compression[4])};
  for corner in 1:4 loop
    suspensionForce[corner] =
      preload[corner] + cornerStiffness * compression[corner]
      + cornerDamping * compressionRate[corner]
      + stopForce[corner] + antiRollForce[corner];
  end for;
  body.verticalForce = suspensionForce;
  body.externalRollMoment = 0;
  body.externalPitchMoment = 0;
  heave = body.heave;
  roll = body.roll;
  pitch = body.pitch;
end FourCornerSuspension;
