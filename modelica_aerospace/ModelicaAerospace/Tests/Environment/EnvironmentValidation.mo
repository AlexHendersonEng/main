within ModelicaAerospace.Tests.Environment;
model EnvironmentValidation "Evaluate atmosphere and gravity at a reference point"
  parameter Real altitude = 0;
  parameter Real latitude = 0;
  ModelicaAerospace.Environment.Atmosphere.Blocks.StandardAtmosphere atmosphereBlock;
  ModelicaAerospace.Environment.Atmosphere.Blocks.AirData airData;
  ModelicaAerospace.Environment.Gravity.Blocks.NormalGravity gravityBlock;
  output Real temperature;
  output Real pressure;
  output Real density;
  output Real speedOfSound;
  output Real dynamicViscosity;
  output Real geopotentialAltitude;
  output Real mach;
  output Real dynamicPressure;
  output Real inverseSquareGravity;
  output Real normalGravity;
  output Real constantGravity[3];
  output Real transportVelocity[3];
  output Real centrifugalAcceleration[3];
  output Real blockTemperature;
  output Real blockGravity[3];
protected
  Real probe;
equation
  probe = 1e-6 * time;
  temperature = atmosphereBlock.temperature + probe;
  pressure = atmosphereBlock.pressure + probe;
  density = atmosphereBlock.density + probe;
  speedOfSound = atmosphereBlock.speedOfSound + probe;
  dynamicViscosity = atmosphereBlock.dynamicViscosity + probe;
  geopotentialAltitude =
    ModelicaAerospace.Environment.Atmosphere.geopotentialAltitude(altitude) + probe;
  mach = airData.mach + probe;
  dynamicPressure = airData.dynamicPressure + probe;
  inverseSquareGravity =
    ModelicaAerospace.Environment.Gravity.inverseSquareGravity(altitude) + probe;
  normalGravity =
    ModelicaAerospace.Environment.Gravity.normalGravity(latitude, altitude) + probe;
  constantGravity =
    ModelicaAerospace.Environment.Gravity.constantGravityNED(9.80665)
    + {probe, 2 * probe, 3 * probe};
  transportVelocity =
    ModelicaAerospace.Environment.Gravity.earthRotationVelocity({6378137, 0, 0})
    + {probe, 2 * probe, 3 * probe};
  centrifugalAcceleration =
    ModelicaAerospace.Environment.Gravity.centrifugalAcceleration({6378137, 0, 0})
    + {probe, 2 * probe, 3 * probe};

  atmosphereBlock.altitude = altitude;
  blockTemperature = atmosphereBlock.temperature + probe;
  airData.trueAirspeed = 250;
  airData.density = atmosphereBlock.density;
  airData.speedOfSound = atmosphereBlock.speedOfSound;
  gravityBlock.latitude = latitude;
  gravityBlock.altitude = altitude;
  blockGravity = gravityBlock.gravityNED + {probe, 2 * probe, 3 * probe};
  assert(abs(airData.mach - (mach - probe)) < 1e-12, "Air-data block Mach mismatch");
  assert(
    abs(airData.dynamicPressure - (dynamicPressure - probe)) < 1e-9,
    "Air-data block dynamic-pressure mismatch");
end EnvironmentValidation;
