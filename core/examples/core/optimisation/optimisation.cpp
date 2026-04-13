#include <cmath>
#include <iostream>
#include <vector>

#include "optimisers/gradient_descent.hpp"
#include "optimisers/particle_swarm.hpp"

void ConvergenceStatistics(std::string name, vanta::optimisers::Solution sol) {
  std::cout << name << " did " << (sol.converged ? "" : "not ")
            << "converge in " << sol.iters << " iterations with value: ";
  for (auto x : sol.x) std::cout << x << ", ";
  std::cout << std::endl;
}

int main() {
  // Define function
  auto f = [](const std::vector<double>& x) {
    // y = (x0 - 3) ^ 2 + (x1 + 2) ^ 2
    return std::pow(x[0] - 3.0, 2) + std::pow(x[1] + 2.0, 2);
  };

  // Define analytic gradient
  auto grad_f = [](const std::vector<double>& x) {
    // ∂y/∂x0 = 2 * (x0 - 3)
    // ∂y/∂x1 = 2 * (x1 + 2)
    return std::vector<double>{2.0 * (x[0] - 3.0), 2.0 * (x[1] + 2.0)};
  };

  // Initial guess
  std::vector<double> x0 = {0.0, 0.0};

  // Gradient descent options
  vanta::optimisers::GDOptions gd_opts{.learning_rate = 0.1,
                                       .max_iters = 1000,
                                       .tolerance = 1e-6,
                                       .finite_difference_step = 1e-6};

  // Run gradient descent optimiser with analystic gradient function
  vanta::optimisers::Solution gd_sol1 =
      vanta::optimisers::GradientDescent(f, x0, grad_f, gd_opts);

  // Convergence check
  ConvergenceStatistics("GradientDescent with analytic gradient function",
                        gd_sol1);

  // Run gradient descent optimiser with finite difference gradient
  // approximation
  vanta::optimisers::Solution gd_sol2 =
      vanta::optimisers::GradientDescent(f, x0, nullptr, gd_opts);

  // Convergence check
  ConvergenceStatistics(
      "GradientDescent with finite difference gradient approximation", gd_sol1);

  // Particle swarm options
  vanta::optimisers::PSOptions ps_opts{
      .n_particles = 100,
      .max_iters = 1000,
      .w = 0.7,
      .c1 = 1.5,
      .c2 = 1.5,
      .tolerance = 1e-6,
      .lower_bounds = std::vector<double>{-10.0, -10.0},
      .upper_bounds = std::vector<double>{10.0, 10.0}};

  // Particle swarm optimiser
  vanta::optimisers::Solution ps_sol =
      vanta::optimisers::ParticleSwarm(2, f, ps_opts);

  // Convergence check
  ConvergenceStatistics("ParticleSwarm", ps_sol);

  return 0;
}
