within ModelicaAutomotive.Interfaces;
partial block PartialTire "Common tire slip-to-force interface"
  ModelicaAutomotive.Interfaces.RealInput slipRatio
    "Positive when wheel circumferential speed exceeds forward speed";
  ModelicaAutomotive.Interfaces.RealInput slipAngle(unit="rad")
    "Positive slip angle produces positive leftward force";
  ModelicaAutomotive.Interfaces.RealInput normalLoad(unit="N")
    "Positive tire normal load";
  ModelicaAutomotive.Interfaces.RealInput frictionCoefficient
    "Tire-road friction coefficient";
  ModelicaAutomotive.Interfaces.RealOutput longitudinalForce(unit="N");
  ModelicaAutomotive.Interfaces.RealOutput lateralForce(unit="N");
  ModelicaAutomotive.Interfaces.RealOutput utilization
    "Combined force divided by the friction limit";
end PartialTire;
