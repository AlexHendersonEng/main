#include "auto_grad/auto_grad.hpp"

#include <iostream>
#include <vector>

int main() {
  // Setup variables
  core::auto_grad::AutoGrad a{2.0};
  core::auto_grad::AutoGrad b{3.0};

  // Perform operation
  auto c = a + b;

  // Compute gradients
  c.backward();

  // Print out auto grad objects
  std::cout << "c: " << c << std::endl;
  std::cout << "b: " << b << std::endl;
  std::cout << "a: " << a << std::endl;
}
