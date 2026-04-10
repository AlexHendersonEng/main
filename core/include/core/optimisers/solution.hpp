#ifndef CORE_OPTIMISERS_SOLUTION_HPP_
#define CORE_OPTIMISERS_SOLUTION_HPP_

#include <vector>

namespace vanta::optimisers {

struct Solution {
  double f_val;
  std::vector<double> x;
  bool converged;
  int iters;
};

}  // namespace vanta::optimisers

#endif  // CORE_OPTIMISERS_SOLUTION_HPP_
