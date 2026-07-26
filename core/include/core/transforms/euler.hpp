#ifndef CORE_TRANSFORMS_EULER_HPP_
#define CORE_TRANSFORMS_EULER_HPP_

#include <array>
#include <iostream>

namespace core::transforms {

std::array<std::array<double, 3>, 3> RotXToDCM(double angle);

}  // namespace core::transforms

std::ostream& operator<<(std::ostream& os,
                         const std::array<std::array<double, 3>, 3>& R);

#endif  // CORE_TRANSFORMS_EULER_HPP_
