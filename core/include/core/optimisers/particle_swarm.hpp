#ifndef CORE_OPTIMISERS_PARTICLE_SWARM_HPP_
#define CORE_OPTIMISERS_PARTICLE_SWARM_HPP_

/**
 * @file particle_swarm.hpp
 * @brief Particle Swarm Optimisation (PSO) algorithm for continuous functions.
 *
 * This header defines a particle swarm optimisation algorithm for
 * minimising scalar-valued objective functions over bounded domains.
 */

#include <functional>
#include <vector>

#include "finite_difference/forward_difference.hpp"
#include "optimisers/solution.hpp"

namespace vanta::optimisers {

/**
 * @brief Configuration options for particle swarm optimisation.
 *
 * This structure contains parameters controlling the behaviour
 * of the PSO algorithm.
 */
struct PSOptions {
  /// Number of particles in the swarm.
  int n_particles = 30;

  /// Maximum number of iterations.
  int max_iters = 1000;

  /// Inertia weight controlling momentum of particles.
  double w = 0.7;

  /// Cognitive coefficient (attraction toward personal best).
  double c1 = 1.5;

  /// Social coefficient (attraction toward global best).
  double c2 = 1.5;

  /// Convergence tolerance on objective function value.
  double tolerance = 1e-6;
};

/**
 * @brief Minimise a function using particle swarm optimisation.
 *
 * This function applies the PSO algorithm to minimise the objective
 * function @p f over a bounded domain defined by @p lower_bounds
 * and @p upper_bounds.
 *
 * The algorithm maintains a swarm of particles that explore the
 * search space by updating their velocities and positions based on:
 * - Inertia (previous velocity)
 * - Cognitive component (personal best)
 * - Social component (global best)
 *
 * Particle positions are clamped to the provided bounds after each update.
 *
 * @param f Objective function to minimise.
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
 * @note Convergence is determined solely by the objective value falling
 *       below @p opts.tolerance.
 * @note Random number generation is handled internally.
 */
vanta::optimisers::Solution ParticleSwarm(
    const std::function<double(const std::vector<double>&)>& f,
    const std::vector<double>& lower_bounds,
    const std::vector<double>& upper_bounds,
    PSOptions opts = {});

}  // namespace vanta::optimisers

#endif  // CORE_OPTIMISERS_PARTICLE_SWARM_HPP_