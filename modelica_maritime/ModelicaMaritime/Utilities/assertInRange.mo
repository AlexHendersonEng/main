within ModelicaMaritime.Utilities;
function assertInRange "Return a value after asserting that it is within a closed interval"
  input Real value;
  input Real minimum;
  input Real maximum;
  input String name = "value";
  output Real checked;
algorithm
  assert(minimum <= maximum, "minimum must not exceed maximum");
  assert(
    value >= minimum and value <= maximum,
    name + " must be within the requested range");
  checked := value;
end assertInRange;
