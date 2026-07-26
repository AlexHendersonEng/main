#include <array>
#include <iostream>
#include <numbers>

#include "transforms/euler.hpp"
#include "transforms/quaternion.hpp"

int main() {
  const auto DCM = core::transforms::EulerToDCM(
      0.0, 0.0, std::numbers::pi / 2.0, core::transforms::RotationOrder::XYZ);
  const auto q = core::transforms::EulerToQuaternion(
      0.0, 0.0, std::numbers::pi / 2.0, core::transforms::RotationOrder::XYZ);
  constexpr std::array<double, 3> vector = {1.0, 0.0, 0.0};
  const auto rotated_vector_by_DCM =
      core::transforms::RotateVectorByDCM(DCM, vector);
  const auto rotated_vector_by_quaternion =
      core::transforms::RotateVectorByQuaternion(q, vector);

  std::cout << "Rotation matrix:\n" << DCM << '\n';
  std::cout << "Rotated vector by DCM: [ " << rotated_vector_by_DCM[0] << ", "
            << rotated_vector_by_DCM[1] << ", " << rotated_vector_by_DCM[2]
            << " ]\n";
  std::cout << "Quaternion: [ " << q[0] << ", " << q[1] << ", " << q[2] << ", "
            << q[3] << " ]\n";
  std::cout << "Rotated vector by quaternion: [ "
            << rotated_vector_by_quaternion[0] << ", "
            << rotated_vector_by_quaternion[1] << ", "
            << rotated_vector_by_quaternion[2] << " ]\n";
}
