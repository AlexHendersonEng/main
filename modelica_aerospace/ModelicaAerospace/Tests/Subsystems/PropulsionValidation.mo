within ModelicaAerospace.Tests.Subsystems;
model PropulsionValidation "Exercise thrust, spool, propeller, jet, and fuel models"
  ModelicaAerospace.Propulsion.ThrustSource source(
    maximumThrust=1000,
    directionBody={1, 1, 0});
  ModelicaAerospace.Propulsion.FirstOrderEngine engine(
    timeConstant=0.5,
    maximumThrust=2000,
    maximumFuelFlow=0.2,
    initialSpool=0);
  ModelicaAerospace.Propulsion.FuelTank tank(initialFuelMass=10);
  ModelicaAerospace.Propulsion.Propeller propeller(
    maximumShaftPower=100000,
    efficiency=0.8,
    minimumAirspeed=10);
  ModelicaAerospace.Propulsion.JetThrust jet(
    seaLevelStaticThrust=10000,
    densityExponent=0.7,
    machLapse=0.25);
  output Real sourceForce[3];
  output Real spool;
  output Real engineThrust;
  output Real fuelFlow;
  output Real fuelMass;
  output Real consumedFuelMass;
  output Real propellerThrust;
  output Real jetThrust;
equation
  source.throttle = 1.2;
  source.enabled = true;
  sourceForce = source.forceBody;
  engine.throttle = if time < 1 then 0 else 0.8;
  engine.enabled = true;
  spool = engine.spool;
  engineThrust = engine.thrust;
  fuelFlow = engine.fuelFlow;
  tank.requestedFuelFlow = engine.fuelFlow;
  fuelMass = tank.fuelMass;
  consumedFuelMass = tank.consumedFuelMass;
  propeller.throttle = 0.5;
  propeller.airspeed = 50;
  propellerThrust = propeller.thrust;
  jet.throttle = 0.75;
  jet.density = 0.6125;
  jet.mach = 0.8;
  jetThrust = jet.thrust;
end PropulsionValidation;
