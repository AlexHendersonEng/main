#ifndef CORE_RUNGE_KUTTA_4_HPP_
#define CORE_RUNGE_KUTTA_4_HPP_

#include <functional>
#include <vector>

#include "block_sim/integration_methods/integration_method.hpp"

namespace core::block_sim {

class RungeKutta4 : public IntegrationMethod {
 public:
  RungeKutta4() = default;

  double integrate(
      std::vector<double>& states,
      const std::function<std::vector<double>&(std::vector<double>&, double)>&
          compute_derivatives,
      double t, double requested_dt) override;
};

}  // namespace core::block_sim

#endif  // CORE_RUNGE_KUTTA_4_HPP_
