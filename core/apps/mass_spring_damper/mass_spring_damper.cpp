#include <iostream>

#include "solvers/ode.h"
#include "utils/utils.h"

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

  // Run simulation
  Solution solution = runge_kutta_4(f, t0, t1, y0, h);

  // Write simulation data to csv file
  to_csv("output.csv", solution.t, solution.y);

  return 0;
}
