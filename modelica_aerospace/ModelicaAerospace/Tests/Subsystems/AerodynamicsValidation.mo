within ModelicaAerospace.Tests.Subsystems;
model AerodynamicsValidation "Exercise aerodynamic scaling, derivatives, and tables"
  ModelicaAerospace.Aerodynamics.CoefficientForcesMoments scaling(
    referenceArea=10,
    referenceSpan=8,
    referenceChord=2);
  ModelicaAerospace.Aerodynamics.StabilityDerivatives derivatives(
    forceBase={-0.02, 0, 0},
    momentBase={0, 0, 0},
    forceAlpha={-0.1, 0, -4},
    forceBeta={0, -0.8, 0},
    momentAlpha={0, -1, 0},
    momentBeta={-0.1, 0, 0.2},
    forceRate=[0.1, 0, 0; 0, 0.2, 0; 0, 0, 0.3],
    momentRate=[0.4, 0, 0; 0, 0.5, 0; 0, 0, 0.6],
    forceControl=[0, 0.2, 0; 0.1, 0, 0.3; 0, -0.5, 0],
    momentControl=[0.8, 0, 0.1; 0, -1.2, 0; -0.1, 0, -0.7],
    referenceSpan=8,
    referenceChord=2);
  output Real forceBody[3];
  output Real momentBody[3];
  output Real derivativeForce[3];
  output Real derivativeMoment[3];
equation
  scaling.dynamicPressure = 500;
  scaling.forceCoefficients = {-0.1, 0.02, -0.5};
  scaling.momentCoefficients = {0.01, -0.03, 0.02};
  forceBody = scaling.forceBody;
  momentBody = scaling.momentBody;

  derivatives.angleOfAttack = 0.1;
  derivatives.sideslip = -0.05;
  derivatives.airspeed = 100;
  derivatives.angularVelocityBody = {0.1, 0.2, -0.1};
  derivatives.controlDeflection = {0.05, -0.1, 0.02};
  derivativeForce = derivatives.forceCoefficients;
  derivativeMoment = derivatives.momentCoefficients;
end AerodynamicsValidation;
