within ModelicaMaritime.Tests.Environment;
model EnvironmentValidation "Validate linear water properties and deterministic currents"
  ModelicaMaritime.Environment.Water.Blocks.ConstantWater constantWater(
    waterTemperature=290,
    waterDensity=1027,
    currentNED={1, -0.5, 0.1});
  ModelicaMaritime.Environment.Water.Blocks.LinearWater linearWater(
    surfaceTemperature=290,
    temperatureGradient=-0.02,
    surfaceSalinity=0.034,
    salinityGradient=2e-5,
    referenceDensity=1024,
    thermalExpansion=2e-4,
    halineContraction=0.8,
    surfaceCurrentNED={1, 0.5, 0},
    currentGradientNED={-0.01, 0.02, 0.001});
  ModelicaMaritime.Environment.Current.Blocks.SteadyCurrent steadyBlock(
    currentNED={2, -1, 0.2});
  ModelicaMaritime.Environment.Current.Blocks.LinearCurrent linearBlock(
    surfaceCurrentNED={1, 0.5, 0},
    gradientNED={-0.01, 0.02, 0.001});
  output Real functionDensity;
  output Real functionPressure;
  output Real directionalCurrent[3];
  output Real profileCurrent[3];
  output Real constantTemperature;
  output Real constantPressure;
  output Real constantDensity;
  output Real constantCurrent[3];
  output Real linearTemperature;
  output Real linearPressure;
  output Real linearDensity;
  output Real linearCurrent[3];
  output Real steadyBlockCurrent[3];
  output Real linearBlockCurrent[3];
equation
  functionDensity = ModelicaMaritime.Environment.Water.densityLinear(
    283.15,
    0.036);
  functionPressure = ModelicaMaritime.Environment.Water.hydrostaticPressure(
    50,
    1025);
  directionalCurrent = ModelicaMaritime.Environment.Current.steadyCurrentNED(
    2,
    0.5235987755982989,
    0.1);
  profileCurrent = ModelicaMaritime.Environment.Current.linearCurrentProfile(
    20,
    {1, 0.5, 0},
    {-0.01, 0.02, 0.001});
  constantWater.depth = 20;
  constantTemperature = constantWater.temperature;
  constantPressure = constantWater.pressure;
  constantDensity = constantWater.density;
  constantCurrent = constantWater.currentVelocityNED;
  linearWater.depth = 20;
  linearTemperature = linearWater.temperature;
  linearPressure = linearWater.pressure;
  linearDensity = linearWater.density;
  linearCurrent = linearWater.currentVelocityNED;
  steadyBlockCurrent = steadyBlock.velocityNED;
  linearBlock.depth = 20;
  linearBlockCurrent = linearBlock.velocityNED;
end EnvironmentValidation;
