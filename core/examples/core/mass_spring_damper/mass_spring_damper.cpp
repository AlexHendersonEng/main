#include <iostream>

#include "ode/euler_backward.hpp"
#include "ode/euler_forward.hpp"
#include "ode/runge_kutta_4.hpp"
#include "utils/output.hpp"

int main() {
  // Mass spring damper function
  std::function<std::vector<double>(const double&, const std::vector<double>&)>
      f = [](const double& t [[maybe_unused]], const std::vector<double>& y) {
        const double k = 0.2;
        const double c = 0.2;

        std::vector<double> dydt{y[1], -c * y[1] - k * y[0]};

        return dydt;
      };

  // Variables
  const double t0 = 0.0;
  const double t1 = 100.0;
  const std::vector<double> y0{1.0, 0.0};
  const double h = 0.1;

  // Run simulations
  vanta::ode::Solution euler_forward_sol =
      vanta::ode::EulerForward(f, t0, t1, y0, h);
  vanta::ode::Solution runge_kutta_4_sol =
      vanta::ode::RungeKutta4(f, t0, t1, y0, h);
  vanta::ode::Solution euler_backward_sol =
      vanta::ode::EulerBackward(f, t0, t1, y0, h);

  // Write simulation data to csv file
  vanta::utils::ToCSV("euler_forward.csv", euler_forward_sol.t,
                      euler_forward_sol.y);
  vanta::utils::ToCSV("runge_kutta_4.csv", runge_kutta_4_sol.t,
                      runge_kutta_4_sol.y);
  vanta::utils::ToCSV("euler_backward.csv", euler_backward_sol.t,
                      euler_backward_sol.y);

  return 0;
}
