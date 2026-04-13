#ifndef CORE_UTILS_RANDOM_HPP_
#define CORE_UTILS_RANDOM_HPP_

#include <random>

namespace vanta::utils {

double RandUniform() {
  static std::mt19937 gen(std::random_device{}());
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  return dist(gen);
}

}  // namespace vanta::utils

#endif  // CORE_UTILS_RANDOM_HPP_
