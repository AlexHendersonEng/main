#include "transforms/euler.hpp"

#include <cmath>

namespace core::transforms {

std::array<std::array<double, 3>, 3> RotXToDCM(double angle) {
  const double c = std::cos(angle);
  const double s = std::sin(angle);

  return {{{1.0, 0.0, 0.0}, {0.0, c, -s}, {0.0, s, c}}};
};

}  // namespace core::transforms

std::ostream& operator<<(std::ostream& os,
                         const std::array<std::array<double, 3>, 3>& R) {
  for (const auto& row : R) {
    os << "[ ";
    for (const double value : row) {
      os << value << " ";
    }
    os << "]\n";
  }

  return os;
}
