#include <cmath>
#include <iostream>
#include <vector>

#include "optimisers/genetic_algorithm.hpp"
#include "optimisers/gradient_descent.hpp"
#include "optimisers/particle_swarm.hpp"

void ConvergenceStatistics(std::string name, core::optimisers::Solution sol) {
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
  core::optimisers::GDOptions gd_opts{.learning_rate = 0.1,
                                      .max_iters = 1000,
                                      .tolerance = 1e-6,
                                      .finite_difference_step = 1e-6};

  // Run gradient descent optimiser with analystic gradient function
  core::optimisers::Solution gd_sol1 =
      core::optimisers::GradientDescent(f, x0, grad_f, gd_opts);

  // Convergence check
  ConvergenceStatistics("GradientDescent with analytic gradient function",
                        gd_sol1);

  // Run gradient descent optimiser with finite difference gradient
  // approximation
  core::optimisers::Solution gd_sol2 =
      core::optimisers::GradientDescent(f, x0, nullptr, gd_opts);

  // Convergence check
  ConvergenceStatistics(
      "GradientDescent with finite difference gradient approximation", gd_sol1);

  // Particle swarm options
  core::optimisers::PSOptions ps_opts{.n_particles = 100,
                                      .max_iters = 1000,
                                      .w = 0.7,
                                      .c1 = 1.5,
                                      .c2 = 1.5,
                                      .tolerance = 1e-6};

  // Particle swarm optimiser
  core::optimisers::Solution ps_sol =
      core::optimisers::ParticleSwarm(f, std::vector<double>{-10.0, -10.0},
                                      std::vector<double>{10.0, 10.0}, ps_opts);

  // Convergence check
  ConvergenceStatistics("ParticleSwarm", ps_sol);

  // Genetic algorithm options
  core::optimisers::GAOptions ga_opts{.population_size = 50,
                                      .max_generations = 500,
                                      .crossover_rate = 0.8,
                                      .mutation_rate = 0.1,
                                      .mutation_strength = 0.1,
                                      .tournament_size = 3,
                                      .tolerance = 1e-6};

  // Genetic algorithm optimiser
  core::optimisers::Solution ga_sol = core::optimisers::GeneticAlgorithm(
      f, std::vector<double>{-10.0, -10.0}, std::vector<double>{10.0, 10.0},
      ga_opts);

  // Convergence check
  ConvergenceStatistics("GeneticAlgotithm", ga_sol);

  return 0;
}
