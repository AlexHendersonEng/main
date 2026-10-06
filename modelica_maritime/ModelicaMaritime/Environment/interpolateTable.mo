within ModelicaMaritime.Environment;
function interpolateTable "Linearly interpolate a scalar table with endpoint clamping"
  input Real abscissa;
  input Real grid[:];
  input Real values[size(grid, 1)];
  output Real value;
protected
  Real fraction;
algorithm
  assert(size(grid, 1) >= 2, "Interpolation table requires at least two rows");
  for index in 2:size(grid, 1) loop
    assert(grid[index] > grid[index - 1], "Interpolation grid must be strictly increasing");
  end for;
  value := values[1];
  for index in 1:size(grid, 1) - 1 loop
    fraction := min(
      1,
      max(0, (abscissa - grid[index]) / (grid[index + 1] - grid[index])));
    value := value + (values[index + 1] - values[index]) * fraction;
  end for;
end interpolateTable;
