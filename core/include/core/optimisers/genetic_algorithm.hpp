#ifndef CORE_OPTIMISERS_GENETIC_ALGORITHM_HPP_
#define CORE_OPTIMISERS_GENETIC_ALGORITHM_HPP_

/**
 * @file genetic_algorithm.hpp
 * @brief Genetic algorithm optimiser for continuous functions.
 *
 * This header defines a configurable genetic algorithm (GA) for
 * minimising objective functions over bounded continuous domains.
 */

#include <functional>
#include <vector>

#include "finite_difference/forward_difference.hpp"
#include "optimisers/solution.hpp"

namespace vanta::optimisers {

/**
 * @brief Configuration options for the genetic algorithm.
 *
 * This structure encapsulates all tunable parameters controlling
 * the behaviour of the genetic algorithm.
 */
struct GAOptions {
  /// Population size (number of individuals per generation).
  int population_size = 50;

  /// Maximum number of generations to evolve.
  int max_generations = 500;

  /// Probability of performing crossover between two parents.
  double crossover_rate = 0.8;

  /// Probability of mutating each gene.
  double mutation_rate = 0.1;

  /// Magnitude of mutation relative to variable range.
  double mutation_strength = 0.1;

  /// Number of individuals competing in tournament selection.
  int tournament_size = 3;

  /// Convergence tolerance on objective function value.
  double tolerance = 1e-6;
};

/**
 * @brief Minimise a function using a genetic algorithm.
 *
 * This function applies a real-valued genetic algorithm to minimise
 * the objective function @p f over a bounded domain defined by
 * @p lower_bounds and @p upper_bounds.
 *
 * The algorithm maintains a population of candidate solutions and
 * evolves them over multiple generations using:
 * - Tournament selection
 * - BLX-α crossover
 * - Random mutation with clamping to bounds
 * - Elitism (best individual preserved each generation)
 *
 * @param f Objective function to minimise. Takes a vector of parameters
 *          and returns a scalar fitness value.
 * @param lower_bounds Lower bounds for each dimension.
 * @param upper_bounds Upper bounds for each dimension.
 * @param opts Configuration parameters for the algorithm (optional).
 *
 * @return A @ref vanta::optimisers::Solution containing:
 *         - Best solution found
 *         - Objective value
 *         - Convergence status
 *         - Number of iterations performed
 *
 * @pre @p lower_bounds and @p upper_bounds must have the same size.
 * @pre Each lower bound must be less than or equal to its corresponding upper bound.
 *
 * @note The algorithm assumes a minimisation problem.
 * @note Random number generation is internally handled and not seeded externally.
 * @note Convergence is determined solely by the objective value falling below
 *       @p opts.tolerance.
 */
vanta::optimisers::Solution GeneticAlgorithm(
    const std::function<double(const std::vector<double>&)>& f,
    const std::vector<double>& lower_bounds,
    const std::vector<double>& upper_bounds,
    GAOptions opts = {});

}  // namespace vanta::optimisers

#endif  // CORE_OPTIMISERS_GENETIC_ALGORITHM_HPP_