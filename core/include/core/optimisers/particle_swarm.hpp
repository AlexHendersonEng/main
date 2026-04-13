#ifndef CORE_OPTIMISERS_PARTICLE_SWARM_HPP_
#define CORE_OPTIMISERS_PARTICLE_SWARM_HPP_

#include <functional>
#include <vector>

#include "finite_difference/forward_difference.hpp"
#include "optimisers/solution.hpp"

namespace vanta::optimisers {

struct PSOptions {
  int n_particles = 30;
  int max_iters = 1000;
  double w = 0.7;   // inertia
  double c1 = 1.5;  // cognitive
  double c2 = 1.5;  // social
  double tolerance = 1e-6;
  std::vector<double> lower_bounds;
  std::vector<double> upper_bounds;
};

struct Particle {
  std::vector<double> position;
  std::vector<double> velocity;
  std::vector<double> best_position;
  double best_value;
};

vanta::optimisers::Solution ParticleSwarm(
    int n_vars, const std::function<double(const std::vector<double>&)>& f,
    PSOptions opts = {});

}  // namespace vanta::optimisers

#endif  // CORE_OPTIMISERS_PARTICLE_SWARM_HPP_
