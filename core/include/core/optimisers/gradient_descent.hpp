#ifndef CORE_OPTIMISERS_GRADIENT_DESCENT_HPP_
#define CORE_OPTIMISERS_GRADIENT_DESCENT_HPP_

#include <functional>
#include <vector>

#include "finite_difference/forward_difference.hpp"
#include "optimisers/solution.hpp"

namespace vanta::optimisers {

struct GDOptions {
  double learning_rate = 0.01;
  int max_iters = 1000;
  double tolerance = 1e-6;
  double finite_difference_step = 1e-6;
  std::vector<double> lower_bounds;
  std::vector<double> upper_bounds;
};

vanta::optimisers::Solution GradientDescent(
    const std::function<double(const std::vector<double>&)>& f,
    std::vector<double> x,
    std::function<std::vector<double>(const std::vector<double>&)> grad_f =
        nullptr,
    GDOptions opts = {});

}  // namespace vanta::optimisers

#endif  // CORE_OPTIMISERS_GRADIENT_DESCENT_HPP_
