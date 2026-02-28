#include <iomanip>
#include <iostream>
#include <vector>

#include "root_finders/newton_raphson.h"

int main() {
  // Define example system of equations:
  // f1(x,y) = x^2 + y^2 - 4
  // f2(x,y) = x - y
  auto f = [](const std::vector<double>& x) -> std::vector<double> {
    return {x[0] * x[0] + x[1] * x[1] - 4.0, x[0] - x[1]};
  };

  // Analytical Jacobian
  auto J_f =
      [](const std::vector<double>& x) -> std::vector<std::vector<double>> {
    return {{2.0 * x[0], 2.0 * x[1]}, {1.0, -1.0}};
  };

  // Define initial guess
  std::vector<double> initialGuess = {1.0, 1.5};

  // Solve using analytial Jacobian
  std::cout << "Using analytical Jacobian:\n";
  std::vector<double> sol1 = newton_raphson(f, initialGuess, J_f);

  std::cout << "Solution:\n";
  for (size_t i = 0; i < sol1.size(); ++i)
    std::cout << "x[" << i << "] = " << std::setprecision(10) << sol1[i]
              << "\n";

  // Solve using numerical Jacobian
  std::cout << "\nUsing numerical Jacobian:\n";
  std::vector<double> sol2 = newton_raphson(f, initialGuess);

  std::cout << "Solution:\n";
  for (size_t i = 0; i < sol2.size(); ++i)
    std::cout << "x[" << i << "] = " << std::setprecision(10) << sol2[i]
              << "\n";

  return 0;
}
