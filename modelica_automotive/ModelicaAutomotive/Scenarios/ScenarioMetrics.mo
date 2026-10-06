within ModelicaAutomotive.Scenarios;
block ScenarioMetrics "Integrate standardized distance, tracking, yaw, and effort metrics"
  ModelicaAutomotive.Interfaces.RealInput speed(unit="m/s");
  ModelicaAutomotive.Interfaces.RealInput lateralError(unit="m");
  ModelicaAutomotive.Interfaces.RealInput yawRate(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealInput controlEffort;
  ModelicaAutomotive.Interfaces.RealOutput distanceTravelled(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput absoluteLateralErrorIntegral(unit="m.s");
  ModelicaAutomotive.Interfaces.RealOutput squaredYawRateIntegral(unit="rad2/s");
  ModelicaAutomotive.Interfaces.RealOutput squaredControlEffortIntegral(unit="s");
protected
  Real distanceState(start=0, fixed=true, unit="m");
  Real lateralErrorState(start=0, fixed=true, unit="m.s");
  Real yawRateState(start=0, fixed=true, unit="rad2/s");
  Real controlEffortState(start=0, fixed=true, unit="s");
equation
  der(distanceState) = abs(speed);
  der(lateralErrorState) = abs(lateralError);
  der(yawRateState) = yawRate * yawRate;
  der(controlEffortState) = controlEffort * controlEffort;
  distanceTravelled = distanceState;
  absoluteLateralErrorIntegral = lateralErrorState;
  squaredYawRateIntegral = yawRateState;
  squaredControlEffortIntegral = controlEffortState;
end ScenarioMetrics;
