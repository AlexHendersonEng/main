#ifndef CORE_EULER_FORWARD_HPP_
#define CORE_EULER_FORWARD_HPP_

#include <functional>
#include <vector>

#include "block_sim/integration_methods/integration_method.hpp"

namespace core::block_sim {

class EulerForward : public IntegrationMethod {
 public:
  EulerForward() = default;

  double integrate(
      std::vector<double>& states,
      const std::function<std::vector<double>&(std::vector<double>&, double)>&
          compute_derivatives,
      double t, double requested_dt) override;
};

}  // namespace core::block_sim

#endif  // CORE_EULER_FORWARD_HPP_
