within ModelicaAutomotive.Tests.Planar;
model SteeringValidation "Validate steering ratio, dynamics, limits, and Ackermann geometry"
  ModelicaAutomotive.Steering.SteeringRatio ratio(
    ratio=15,
    maximumRoadWheelAngle=0.5);
  ModelicaAutomotive.Steering.FirstOrderSteering dynamic(
    timeConstant=0.5,
    maximumAngle=0.5,
    maximumRate=10);
  ModelicaAutomotive.Steering.Ackermann ackermann;
  ModelicaAutomotive.Steering.Ackermann ackermannNegative;
  output Real ratioAngle;
  output Real dynamicAngle;
  output Real dynamicRate;
  output Real wheelAngles[4];
  output Real ratioIntegral(start=0, fixed=true);
  output Real wheelAngleIntegral[4](each start=0, each fixed=true);
  output Real negativeWheelAngleIntegral[4](each start=0, each fixed=true);
equation
  ratio.steeringWheelAngle = 6;
  dynamic.command = 0.3;
  ackermann.steeringAngle = 0.3;
  ackermannNegative.steeringAngle = -0.3;
  ratioAngle = ratio.roadWheelAngle;
  dynamicAngle = dynamic.angle;
  dynamicRate = dynamic.rate;
  wheelAngles = ackermann.wheelAngles;
  der(ratioIntegral) = ratio.roadWheelAngle;
  der(wheelAngleIntegral) = ackermann.wheelAngles;
  der(negativeWheelAngleIntegral) = ackermannNegative.wheelAngles;
end SteeringValidation;
