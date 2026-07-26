#include "transforms/quaternion.hpp"

#include <cmath>
#include <stdexcept>

namespace {

using Quaternion = core::transforms::Quaternion;

Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs) {
  return {
      lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
      lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
      lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x,
      lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w,
  };
}

Quaternion Conjugate(const Quaternion& quaternion) {
  return {quaternion.w, -quaternion.x, -quaternion.y, -quaternion.z};
}

}  // namespace

namespace core::transforms {

Quaternion RotXToQuaternion(const double angle) {
  const double half_angle = angle / 2.0;
  return {std::cos(half_angle), std::sin(half_angle), 0.0, 0.0};
}

Quaternion RotYToQuaternion(const double angle) {
  const double half_angle = angle / 2.0;
  return {std::cos(half_angle), 0.0, std::sin(half_angle), 0.0};
}

Quaternion RotZToQuaternion(const double angle) {
  const double half_angle = angle / 2.0;
  return {std::cos(half_angle), 0.0, 0.0, std::sin(half_angle)};
}

Quaternion EulerToQuaternion(const double angle1, const double angle2,
                             const double angle3, const RotationOrder order) {
  switch (order) {
    case RotationOrder::XYZ:
      return Multiply(
          Multiply(RotXToQuaternion(angle1), RotYToQuaternion(angle2)),
          RotZToQuaternion(angle3));
    case RotationOrder::XZY:
      return Multiply(
          Multiply(RotXToQuaternion(angle1), RotZToQuaternion(angle2)),
          RotYToQuaternion(angle3));
    case RotationOrder::YXZ:
      return Multiply(
          Multiply(RotYToQuaternion(angle1), RotXToQuaternion(angle2)),
          RotZToQuaternion(angle3));
    case RotationOrder::YZX:
      return Multiply(
          Multiply(RotYToQuaternion(angle1), RotZToQuaternion(angle2)),
          RotXToQuaternion(angle3));
    case RotationOrder::ZXY:
      return Multiply(
          Multiply(RotZToQuaternion(angle1), RotXToQuaternion(angle2)),
          RotYToQuaternion(angle3));
    case RotationOrder::ZYX:
      return Multiply(
          Multiply(RotZToQuaternion(angle1), RotYToQuaternion(angle2)),
          RotXToQuaternion(angle3));
  }

  throw std::invalid_argument("Unsupported rotation order.");
}

std::array<double, 3> RotateVectorByQuaternion(
    const Quaternion& rotation, const std::array<double, 3>& vector) {
  const double squared_norm = rotation.w * rotation.w +
                              rotation.x * rotation.x +
                              rotation.y * rotation.y + rotation.z * rotation.z;
  if (squared_norm == 0.0) {
    throw std::invalid_argument("Cannot rotate with a zero quaternion.");
  }

  const Quaternion vector_quaternion = {0.0, vector[0], vector[1], vector[2]};
  const Quaternion rotated =
      Multiply(Multiply(Conjugate(rotation), vector_quaternion), rotation);

  return {rotated.x / squared_norm, rotated.y / squared_norm,
          rotated.z / squared_norm};
}

}  // namespace core::transforms
