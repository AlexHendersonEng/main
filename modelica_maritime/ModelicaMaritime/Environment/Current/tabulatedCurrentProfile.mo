within ModelicaMaritime.Environment.Current;
function tabulatedCurrentProfile "Interpolate a tabulated NED current profile"
  input ModelicaMaritime.Types.Length depth "Depth below surface, positive down";
  input ModelicaMaritime.Types.Length depthGrid[:];
  input ModelicaMaritime.Types.Velocity currentTable[size(depthGrid, 1), 3];
  output ModelicaMaritime.Types.Velocity currentNED[3];
protected
  Real fraction;
algorithm
  assert(depth >= 0, "Current profile requires non-negative depth");
  assert(size(depthGrid, 1) >= 2, "Current profile requires at least two rows");
  for index in 2:size(depthGrid, 1) loop
    assert(
      depthGrid[index] > depthGrid[index - 1],
      "Current-profile depths must be strictly increasing");
  end for;
  for axis in 1:3 loop
    currentNED[axis] := currentTable[1, axis];
    for index in 1:size(depthGrid, 1) - 1 loop
      fraction := min(
        1,
        max(
          0,
          (depth - depthGrid[index])
            / (depthGrid[index + 1] - depthGrid[index])));
      currentNED[axis] := currentNED[axis]
        + (currentTable[index + 1, axis] - currentTable[index, axis]) * fraction;
    end for;
  end for;
end tabulatedCurrentProfile;
