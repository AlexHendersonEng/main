#ifndef CORE_TRANSFORMS_QUATERNION_HPP_
#define CORE_TRANSFORMS_QUATERNION_HPP_

#include <array>
#include <cstddef>
#include <stdexcept>

#include "transforms/euler.hpp"

namespace core::transforms {

struct Quaternion {
  double w;
  double x;
  double y;
  double z;

  double& operator[](const std::size_t index) {
    switch (index) {
      case 0:
        return w;
      case 1:
        return x;
      case 2:
        return y;
      case 3:
        return z;
      default:
        throw std::out_of_range("Quaternion index is out of range.");
    }
  }

  const double& operator[](const std::size_t index) const {
    switch (index) {
      case 0:
        return w;
      case 1:
        return x;
      case 2:
        return y;
      case 3:
        return z;
      default:
        throw std::out_of_range("Quaternion index is out of range.");
    }
  }
};

// Quaternion rotations use the same passive convention as the Euler functions.
Quaternion RotXToQuaternion(double angle);
Quaternion RotYToQuaternion(double angle);
Quaternion RotZToQuaternion(double angle);
Quaternion EulerToQuaternion(double angle1, double angle2, double angle3,
                             RotationOrder order);
std::array<double, 3> RotateVectorByQuaternion(
    const Quaternion& rotation, const std::array<double, 3>& vector);

}  // namespace core::transforms

#endif  // CORE_TRANSFORMS_QUATERNION_HPP_
