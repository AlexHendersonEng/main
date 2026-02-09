#pragma once

#include <functional>

#include "solution.hpp"

Solution euler_forward(const std::function<std::vector<double>(
                           const double&, const std::vector<double>&)>& f,
                       const double& t0, const double& t1,
                       const std::vector<double>& y0, const double& h);
