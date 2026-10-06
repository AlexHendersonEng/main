within ModelicaMaritime.Environment.Current;
function linearCurrentProfile "Evaluate a linear NED current profile at depth"
  input ModelicaMaritime.Types.Length depth "Depth below surface, positive down";
  input ModelicaMaritime.Types.Velocity surfaceCurrentNED[3];
  input Real gradientNED[3](each unit="1/s");
  output ModelicaMaritime.Types.Velocity currentNED[3];
algorithm
  assert(depth >= 0, "Current profile requires non-negative depth");
  currentNED := surfaceCurrentNED + gradientNED * depth;
end linearCurrentProfile;
