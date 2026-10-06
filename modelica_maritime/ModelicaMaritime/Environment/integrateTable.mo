within ModelicaMaritime.Environment;
function integrateTable "Integrate a piecewise-linear table from its first grid point"
  input Real abscissa;
  input Real grid[:];
  input Real values[size(grid, 1)];
  output Real integral;
protected
  Real distance;
  Real clampedDistance;
  Real intervalWidth;
algorithm
  assert(size(grid, 1) >= 2, "Integration table requires at least two rows");
  assert(abscissa >= grid[1], "Integration abscissa must not precede the first grid point");
  for index in 2:size(grid, 1) loop
    assert(grid[index] > grid[index - 1], "Integration grid must be strictly increasing");
  end for;
  integral := values[1] * (abscissa - grid[1]);
  for index in 1:size(grid, 1) - 1 loop
    intervalWidth := grid[index + 1] - grid[index];
    distance := max(abscissa - grid[index], 0);
    clampedDistance := min(distance, intervalWidth);
    integral := integral
      + (values[index + 1] - values[index])
        * (
          clampedDistance * clampedDistance / (2 * intervalWidth)
          + max(distance - intervalWidth, 0));
  end for;
end integrateTable;
