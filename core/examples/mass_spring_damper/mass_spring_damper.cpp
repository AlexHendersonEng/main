#include <iostream>

#include "ode/euler_forward.h"
#include "ode/runge_kutta_4.h"
#include "utils/output.h"

int main() {
  // Mass spring damper function
  std::function<std::vector<double>(const double&, const std::vector<double>&)>
      f = [](const double& t, const std::vector<double>& y) {
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
  Solution euler_forward_sol = euler_forward(f, t0, t1, y0, h);
  Solution runge_kutta_4_sol = runge_kutta_4(f, t0, t1, y0, h);

  // Write simulation data to csv file
  to_csv("euler_forward.csv", euler_forward_sol.t, euler_forward_sol.y);
  to_csv("runge_kutta_4.csv", runge_kutta_4_sol.t, runge_kutta_4_sol.y);

  return 0;
}
