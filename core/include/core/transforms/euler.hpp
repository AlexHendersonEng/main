#ifndef CORE_TRANSFORMS_EULER_HPP_
#define CORE_TRANSFORMS_EULER_HPP_

#include <array>
#include <iostream>

namespace core::transforms {

// Rotation sequences are applied in the listed axis order as passive rotations.
enum class RotationOrder {
  XYZ,
  XZY,
  YXZ,
  YZX,
  ZXY,
  ZYX,
};

// Passive direction cosine matrix from euler angles.
std::array<std::array<double, 3>, 3> RotXToDCM(double angle);
std::array<std::array<double, 3>, 3> RotYToDCM(double angle);
std::array<std::array<double, 3>, 3> RotZToDCM(double angle);
std::array<std::array<double, 3>, 3> EulerToDCM(double angle1, double angle2,
                                                double angle3,
                                                RotationOrder order);
std::array<double, 3> RotateVectorByDCM(
    const std::array<std::array<double, 3>, 3>& rotation_matrix,
    const std::array<double, 3>& vector);

}  // namespace core::transforms

std::ostream& operator<<(std::ostream& os,
                         const std::array<std::array<double, 3>, 3>& R);

#endif  // CORE_TRANSFORMS_EULER_HPP_
