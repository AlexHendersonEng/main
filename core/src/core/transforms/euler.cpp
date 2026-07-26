#include "transforms/euler.hpp"

#include <cmath>
#include <stdexcept>

namespace {

using DCM = std::array<std::array<double, 3>, 3>;

DCM Multiply(const DCM& lhs, const DCM& rhs) {
  DCM result{};

  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      double value = 0.0;
      for (int k = 0; k < 3; ++k) {
        value += lhs[row][k] * rhs[k][col];
      }
      result[row][col] = value;
    }
  }

  return result;
}

}  // namespace

namespace core::transforms {

std::array<std::array<double, 3>, 3> RotXToDCM(const double angle) {
  const double c = std::cos(angle);
  const double s = std::sin(angle);

  return {{{1.0, 0.0, 0.0}, {0.0, c, s}, {0.0, -s, c}}};
};

std::array<std::array<double, 3>, 3> RotYToDCM(const double angle) {
  const double c = std::cos(angle);
  const double s = std::sin(angle);

  return {{{c, 0.0, -s}, {0.0, 1.0, 0.0}, {s, 0.0, c}}};
};

std::array<std::array<double, 3>, 3> RotZToDCM(const double angle) {
  const double c = std::cos(angle);
  const double s = std::sin(angle);

  return {{{c, s, 0.0}, {-s, c, 0.0}, {0.0, 0.0, 1.0}}};
};

std::array<std::array<double, 3>, 3> EulerToDCM(const double angle1,
                                                const double angle2,
                                                const double angle3,
                                                const RotationOrder order) {
  switch (order) {
    case RotationOrder::XYZ:
      return Multiply(Multiply(RotZToDCM(angle3), RotYToDCM(angle2)),
                      RotXToDCM(angle1));
    case RotationOrder::XZY:
      return Multiply(Multiply(RotYToDCM(angle3), RotZToDCM(angle2)),
                      RotXToDCM(angle1));
    case RotationOrder::YXZ:
      return Multiply(Multiply(RotZToDCM(angle3), RotXToDCM(angle2)),
                      RotYToDCM(angle1));
    case RotationOrder::YZX:
      return Multiply(Multiply(RotXToDCM(angle3), RotZToDCM(angle2)),
                      RotYToDCM(angle1));
    case RotationOrder::ZXY:
      return Multiply(Multiply(RotYToDCM(angle3), RotXToDCM(angle2)),
                      RotZToDCM(angle1));
    case RotationOrder::ZYX:
      return Multiply(Multiply(RotXToDCM(angle3), RotYToDCM(angle2)),
                      RotZToDCM(angle1));
  }

  throw std::invalid_argument("Unsupported rotation order.");
}

std::array<double, 3> RotateVectorByDCM(
    const std::array<std::array<double, 3>, 3>& rotation_matrix,
    const std::array<double, 3>& vector) {
  std::array<double, 3> rotated_vector{};

  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      rotated_vector[row] += rotation_matrix[row][col] * vector[col];
    }
  }

  return rotated_vector;
}

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
