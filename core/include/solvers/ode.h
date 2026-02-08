#pragma once

#include <functional>
#include <vector>

struct Solution {
  std::vector<double> t;
  std::vector<std::vector<double>> y;
};

Solution euler_forward(const std::function<std::vector<double>(
                           const double&, const std::vector<double>&)>& f,
                       const double& t0, const double& t1,
                       const std::vector<double>& y0, const double& h);
