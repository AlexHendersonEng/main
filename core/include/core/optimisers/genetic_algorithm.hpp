#ifndef CORE_OPTIMISERS_GENETIC_ALGORITHM_HPP_
#define CORE_OPTIMISERS_GENETIC_ALGORITHM_HPP_

#include <functional>
#include <vector>

#include "finite_difference/forward_difference.hpp"
#include "optimisers/solution.hpp"

namespace vanta::optimisers {

struct GAOptions {
  int population_size = 50;
  int max_generations = 500;
  double crossover_rate = 0.8;
  double mutation_rate = 0.1;
  double mutation_strength = 0.1;
  int tournament_size = 3;
  double tolerance = 1e-6;
};

vanta::optimisers::Solution GeneticAlgorithm(
    const std::function<double(const std::vector<double>&)>& f,
    const std::vector<double>& lower_bounds, const std::vector<double>& upper_bounds,
    GAOptions opts = {});

}  // namespace vanta::optimisers

#endif  // CORE_OPTIMISERS_GENETIC_ALGORITHM_HPP_
