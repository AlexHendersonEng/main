#include "optimisers/particle_swarm.hpp"

#include "utils/math.hpp"
#include "utils/random.hpp"

namespace vanta::optimisers {

vanta::optimisers::Solution ParticleSwarm(
    int n_vars, const std::function<double(const std::vector<double>&)>& f,
    PSOptions opts) {
  // Create swarm
  std::vector<Particle> swarm(opts.n_particles);

  std::vector<double> global_best_position(n_vars);
  double global_best_value = std::numeric_limits<double>::infinity();

  // Initialize swarm
  for (auto& p : swarm) {
    p.position.resize(n_vars);
    p.velocity.resize(n_vars);
    p.best_position.resize(n_vars);

    for (int i = 0; i < n_vars; ++i) {
      p.position[i] = (opts.upper_bounds[i] - opts.lower_bounds[i]) *
                          vanta::utils::RandUniform() -
                      opts.lower_bounds[i];
      p.velocity[i] = 2.0 * vanta::utils::RandUniform() - 1.0;
    }

    p.best_position = p.position;
    p.best_value = f(p.position);

    if (p.best_value < global_best_value) {
      global_best_value = p.best_value;
      global_best_position = p.position;
    }
  }

  // Main loop
  int iter = 0;
  for (; iter < opts.max_iters; ++iter) {
    for (auto& p : swarm) {
      for (int i = 0; i < n_vars; ++i) {
        // Random values
        double r1 = vanta::utils::RandUniform();
        double r2 = vanta::utils::RandUniform();

        // Velocity update
        p.velocity[i] =
            opts.w * p.velocity[i] +
            opts.c1 * r1 * (p.best_position[i] - p.position[i]) +
            opts.c2 * r2 * (global_best_position[i] - p.position[i]);

        // Position update
        p.position[i] += p.velocity[i];

        // Clamp to bounds
        p.position[i] = vanta::utils::Clamp(p.position[i], opts.lower_bounds[i],
                                            opts.upper_bounds[i]);
      }

      // Assess value
      double value = f(p.position);

      // Update personal best
      if (value < p.best_value) {
        p.best_value = value;
        p.best_position = p.position;
      }

      // Update global best
      if (value < global_best_value) {
        global_best_value = value;
        global_best_position = p.position;
      }
    }

    // Convergence check
    if (global_best_value < opts.tolerance) {
      break;
    }
  }

  // Create solution structure
  vanta::optimisers::Solution sol{
      .f_val = global_best_value,
      .x = global_best_position,
      .converged = global_best_value < opts.tolerance,
      .iters = iter};

  return sol;
}

}  // namespace vanta::optimisers
