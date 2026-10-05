within ModelicaAerospace.Guidance;
block CommandErrors "Heading, altitude, and speed tracking errors"
  ModelicaAerospace.Interfaces.RealInput commandedHeading(unit="rad");
  ModelicaAerospace.Interfaces.RealInput heading(unit="rad");
  ModelicaAerospace.Interfaces.RealInput commandedAltitude(unit="m");
  ModelicaAerospace.Interfaces.RealInput altitude(unit="m");
  ModelicaAerospace.Interfaces.RealInput commandedSpeed(unit="m/s");
  ModelicaAerospace.Interfaces.RealInput speed(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput headingError(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput altitudeError(unit="m");
  ModelicaAerospace.Interfaces.RealOutput speedError(unit="m/s");
equation
  headingError =
    ModelicaAerospace.Mathematics.wrapAngle(commandedHeading - heading);
  altitudeError = commandedAltitude - altitude;
  speedError = commandedSpeed - speed;
end CommandErrors;
