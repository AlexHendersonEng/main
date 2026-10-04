within ModelicaAerospace.Aerodynamics;
block StabilityDerivatives "Linear aerodynamic stability-derivative model"
  parameter Real forceBase[3] = {0, 0, 0};
  parameter Real momentBase[3] = {0, 0, 0};
  parameter Real forceAlpha[3] = {0, 0, 0};
  parameter Real forceBeta[3] = {0, 0, 0};
  parameter Real momentAlpha[3] = {0, 0, 0};
  parameter Real momentBeta[3] = {0, 0, 0};
  parameter Real forceRate[3, 3] = zeros(3, 3)
    "Columns multiply normalized {p, q, r}";
  parameter Real momentRate[3, 3] = zeros(3, 3);
  parameter Real forceControl[3, 3] = zeros(3, 3);
  parameter Real momentControl[3, 3] = zeros(3, 3);
  parameter ModelicaAerospace.Types.Length referenceSpan = 1;
  parameter ModelicaAerospace.Types.Length referenceChord = 1;
  ModelicaAerospace.Interfaces.RealInput angleOfAttack(unit="rad");
  ModelicaAerospace.Interfaces.RealInput sideslip(unit="rad");
  ModelicaAerospace.Interfaces.RealInput airspeed(unit="m/s");
  ModelicaAerospace.Interfaces.Vector3Input angularVelocityBody(each unit="rad/s");
  ModelicaAerospace.Interfaces.Vector3Input controlDeflection(each unit="rad")
    "{aileron, elevator, rudder}";
  ModelicaAerospace.Interfaces.Vector3Output forceCoefficients;
  ModelicaAerospace.Interfaces.Vector3Output momentCoefficients;
protected
  Real normalizedRate[3];
equation
  assert(airspeed > 1e-12, "Stability-derivative airspeed must be positive");
  normalizedRate = {
    angularVelocityBody[1] * referenceSpan / (2 * airspeed),
    angularVelocityBody[2] * referenceChord / (2 * airspeed),
    angularVelocityBody[3] * referenceSpan / (2 * airspeed)};
  forceCoefficients = forceBase + forceAlpha * angleOfAttack
    + forceBeta * sideslip + forceRate * normalizedRate
    + forceControl * controlDeflection;
  momentCoefficients = momentBase + momentAlpha * angleOfAttack
    + momentBeta * sideslip + momentRate * normalizedRate
    + momentControl * controlDeflection;
end StabilityDerivatives;
