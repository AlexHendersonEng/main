within ModelicaAerospace.Sensors;
block IdealInertial "Ideal accelerometer and rate gyro"
  ModelicaAerospace.Interfaces.Vector3Input accelerationBody(each unit="m/s2");
  ModelicaAerospace.Interfaces.Vector3Input angularVelocityBody(each unit="rad/s");
  ModelicaAerospace.Interfaces.Vector3Output measuredAccelerationBody(each unit="m/s2");
  ModelicaAerospace.Interfaces.Vector3Output measuredAngularVelocityBody(each unit="rad/s");
equation
  measuredAccelerationBody = accelerationBody;
  measuredAngularVelocityBody = angularVelocityBody;
end IdealInertial;
