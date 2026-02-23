#include <iostream>
#include <vector>

#include "linear_solvers/gaussian_elimination.h"

int main() {
  // Problem setup
  std::vector<std::vector<double>> A = {{2, 1, -1}, {-3, -1, 2}, {-2, 1, 2}};
  std::vector<double> b = {8, -11, -3};

  // Solve
  std::vector<double> solution = gaussian_elimination(A, b);

  // Print solution
  std::cout << "Solution:\n";
  for (size_t i = 0; i < solution.size(); i++) {
    std::cout << "x" << i << " = " << solution[i] << std::endl;
  }

  return 0;
}
