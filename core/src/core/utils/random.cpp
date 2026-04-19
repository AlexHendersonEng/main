#include "utils/random.hpp"

namespace vanta::utils {

namespace {
std::mt19937& Engine() {
  static std::mt19937 gen{std::random_device{}()};
  return gen;
}
}  // namespace

void SetRandomSeed(unsigned int seed) { Engine().seed(seed); }

double RandUniform() {
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  return dist(Engine());
}

int RandInt(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  return dist(Engine());
}

}  // namespace vanta::utils
