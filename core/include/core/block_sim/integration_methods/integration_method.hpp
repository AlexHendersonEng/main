#ifndef CORE_INTEGRATION_METHOD_HPP_
#define CORE_INTEGRATION_METHOD_HPP_

#include <functional>
#include <vector>

namespace core::block_sim {

class IntegrationMethod {
 public:
  IntegrationMethod() = default;
  virtual ~IntegrationMethod() = default;

  virtual double integrate(
      std::vector<double>& states,
      const std::function<std::vector<double>&(std::vector<double>&, double)>&
          compute_derivatives,
      double t, double requested_dt) = 0;
};

}  // namespace core::block_sim

#endif  // CORE_INTEGRATION_METHOD_HPP_
