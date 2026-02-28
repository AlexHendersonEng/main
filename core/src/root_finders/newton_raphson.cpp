#include "root_finders/newton_raphson.h"

#include <iostream>

#include "finite_difference/forward_difference.h"
#include "linear_solvers/gaussian_elimination.h"
#include "utils/math.h"

std::vector<double> newton_raphson(
    const std::function<std::vector<double>(const std::vector<double>&)>& f,
    const std::vector<double>& x0,
    const std::function<
        std::vector<std::vector<double>>(const std::vector<double>&)>& J_f,
    int maxIter, double tol) {
  // Extract state vector
  std::vector<double> x = x0;

  for (int iter = 0; iter < maxIter; ++iter) {
    // Evaluate function
    std::vector<double> Fx = f(x);

    // Check if converged
    if (vec_norm(Fx) < tol) {
      std::cout << "Converged in " << iter << " iterations.\n";
      return x;
    }

    // Compute Jacobian
    std::vector<std::vector<double>> J;

    if (J_f) {
      // Use user provided Jacobian
      J = J_f(x);
    } else {
      // Use numerical approximation
      J = forward_difference(f, x);
    }

    // Solve J * delta = -F
    for (double& val : Fx) val = -val;

    std::vector<double> delta = gaussian_elimination(J, Fx);

    // Update state vector
    for (size_t i = 0; i < x.size(); ++i) x[i] += delta[i];
  }

  return x;
}
