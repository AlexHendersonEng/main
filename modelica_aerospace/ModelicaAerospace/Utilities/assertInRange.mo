within ModelicaAerospace.Utilities;
function assertInRange "Return a value after asserting inclusive bounds"
  input Real value;
  input Real minimum;
  input Real maximum;
  input String name = "value";
  output Real checked;
algorithm
  assert(minimum <= maximum, "minimum must not exceed maximum");
  assert(value >= minimum and value <= maximum, name + " is outside its valid range");
  checked := value;
end assertInRange;
