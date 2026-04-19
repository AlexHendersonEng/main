#include "utils/random.hpp"

namespace vanta::utils {

double RandUniform() {
  static std::mt19937 gen(std::random_device{}());
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  return dist(gen);
}

int RandInt(int min, int max) {
  static std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<int> dist(min, max);
  return dist(gen);
}

}  // namespace vanta::utils
