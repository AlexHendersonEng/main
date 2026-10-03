#ifndef CORE_FLOW_FLUID_PROPERTIES_HPP_
#define CORE_FLOW_FLUID_PROPERTIES_HPP_

/**
 * @file fluid_properties.hpp
 * @brief Constant fluid properties for incompressible flow.
 */

namespace core::flow {

struct FluidProperties {
  double density = 1.0;               ///< rho [kg/m^3]
  double kinematic_viscosity = 1e-3;  ///< nu [m^2/s]
};

}  // namespace core::flow

#endif  // CORE_FLOW_FLUID_PROPERTIES_HPP_
