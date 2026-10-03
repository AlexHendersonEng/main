#ifndef CORE_FLOW_BOUNDARY_CONDITIONS_HPP_
#define CORE_FLOW_BOUNDARY_CONDITIONS_HPP_

/**
 * @file boundary_conditions.hpp
 * @brief Boundary condition specification for the rectangular domain edges.
 */

namespace core::flow {

enum class BoundaryType {
  kWall,      ///< No-slip wall, tangential velocity = (u, v) of the wall.
  kInlet,     ///< Prescribed velocity (u, v) and turbulence quantities.
  kOutlet,    ///< Prescribed pressure, zero-gradient for other quantities.
  kSymmetry,  ///< Zero normal velocity, zero normal gradients.
};

struct BoundaryCondition {
  BoundaryType type = BoundaryType::kWall;
  double u = 0.0;         ///< Velocity x-component (wall/inlet).
  double v = 0.0;         ///< Velocity y-component (wall/inlet).
  double pressure = 0.0;  ///< Static pressure (outlet).
  double k = 0.0;         ///< Turbulent kinetic energy (inlet).
  double epsilon = 0.0;   ///< Turbulent dissipation rate (inlet).
};

struct BoundaryConditions {
  BoundaryCondition west;
  BoundaryCondition east;
  BoundaryCondition south;
  BoundaryCondition north;
};

}  // namespace core::flow

#endif  // CORE_FLOW_BOUNDARY_CONDITIONS_HPP_
