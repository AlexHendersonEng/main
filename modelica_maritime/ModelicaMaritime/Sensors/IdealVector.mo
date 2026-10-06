within ModelicaMaritime.Sensors;
block IdealVector "Ideal three-axis sensor"
  ModelicaMaritime.Interfaces.Vector3Input truth;
  ModelicaMaritime.Interfaces.Vector3Output measurement;
equation
  measurement = truth;
end IdealVector;
