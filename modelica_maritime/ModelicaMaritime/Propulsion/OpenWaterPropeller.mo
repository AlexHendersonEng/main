within ModelicaMaritime.Propulsion;
block OpenWaterPropeller "Low-order open-water propeller thrust, torque, and power"
  parameter ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter ModelicaMaritime.Types.Length diameter = 1;
  parameter Real thrustCoefficient[3] = {0.2, -0.1, 0}
    "Quadratic KT(J) coefficients";
  parameter Real torqueCoefficient[3] = {0.03, -0.01, 0}
    "Quadratic KQ(J) coefficients";
  parameter Real wakeFraction(min=0, max=1) = 0;
  parameter Real thrustDeduction(min=0, max=1) = 0;
  ModelicaMaritime.Interfaces.RealInput shaftRate(unit="1/s")
    "Signed revolutions per second";
  ModelicaMaritime.Interfaces.RealInput advanceVelocity(unit="m/s");
  ModelicaMaritime.Interfaces.RealOutput advanceRatio;
  ModelicaMaritime.Interfaces.RealOutput thrust(unit="N");
  ModelicaMaritime.Interfaces.RealOutput torque(unit="N.m");
  ModelicaMaritime.Interfaces.RealOutput shaftPower(unit="W");
protected
  Real inflowVelocity(unit="m/s");
  Real coefficientThrust;
  Real coefficientTorque;
equation
  assert(diameter > 0, "Propeller diameter must be positive");
  inflowVelocity = (1 - wakeFraction) * advanceVelocity;
  advanceRatio = if abs(shaftRate) > ModelicaMaritime.Constants.small then
    inflowVelocity / (shaftRate * diameter) else 0;
  coefficientThrust = thrustCoefficient[1]
    + thrustCoefficient[2] * advanceRatio
    + thrustCoefficient[3] * advanceRatio * advanceRatio;
  coefficientTorque = torqueCoefficient[1]
    + torqueCoefficient[2] * advanceRatio
    + torqueCoefficient[3] * advanceRatio * advanceRatio;
  thrust = (1 - thrustDeduction) * density * diameter ^ 4
    * coefficientThrust * shaftRate * abs(shaftRate);
  torque = density * diameter ^ 5
    * coefficientTorque * shaftRate * abs(shaftRate);
  shaftPower = 6.283185307179586 * shaftRate * torque;
end OpenWaterPropeller;
