#include "auto_grad/auto_grad.hpp"

#include <iostream>
#include <vector>

int main() {
  // Setup variables
  core::auto_grad::AutoGrad a{2.0};
  core::auto_grad::AutoGrad b{3.0};
  core::auto_grad::AutoGrad c{4.0};
  core::auto_grad::AutoGrad d{5.0};

  // Perform operation
  auto e = a + b;
  auto f = e - c;
  auto g = f * d;

  // Compute gradients
  g.Backward();

  // Print out auto grad objects
  std::cout << "a: " << a << std::endl;
  std::cout << "b: " << b << std::endl;
  std::cout << "c: " << c << std::endl;
  std::cout << "d: " << d << std::endl;
  std::cout << "e: " << e << std::endl;
  std::cout << "f: " << f << std::endl;
  std::cout << "g: " << g << std::endl;

  // Zero gradients
  g.ZeroGrad();

  // Print out auto grad objects
  std::cout << "a: " << a << std::endl;
  std::cout << "b: " << b << std::endl;
  std::cout << "c: " << c << std::endl;
  std::cout << "d: " << d << std::endl;
  std::cout << "e: " << e << std::endl;
  std::cout << "f: " << f << std::endl;
  std::cout << "g: " << g << std::endl;
}
