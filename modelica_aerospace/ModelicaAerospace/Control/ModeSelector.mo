within ModelicaAerospace.Control;
block ModeSelector "Select between primary and alternate commands using MSL"
  ModelicaAerospace.Interfaces.RealInput primaryCommand;
  ModelicaAerospace.Interfaces.RealInput alternateCommand;
  input Boolean usePrimary;
  ModelicaAerospace.Interfaces.RealOutput command;
protected
  Modelica.Blocks.Logical.Switch selector;
equation
  selector.u1 = primaryCommand;
  selector.u2 = usePrimary;
  selector.u3 = alternateCommand;
  command = selector.y;
end ModeSelector;
