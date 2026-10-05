within ModelicaAutomotive.Utilities;
function assertPositive "Return a value after asserting that it is positive"
  input Real value;
  input String name = "value";
  output Real checked;
algorithm
  assert(value > 0, name + " must be positive");
  checked := value;
end assertPositive;
