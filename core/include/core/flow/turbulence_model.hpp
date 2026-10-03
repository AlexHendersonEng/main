#ifndef CORE_FLOW_TURBULENCE_MODEL_HPP_
#define CORE_FLOW_TURBULENCE_MODEL_HPP_

/**
 * @file turbulence_model.hpp
 * @brief Interface for RANS eddy-viscosity turbulence models.
 */

#include <cstddef>

#include "flow/field.hpp"

namespace core::flow {

/// @brief Turbulence closure selection for the RANS solver.
enum class TurbulenceModelType {
  kLaminar,   ///< No turbulence model, eddy viscosity is zero.
  kKEpsilon,  ///< Standard k-epsilon model with wall functions.
};

/// @brief Eddy-viscosity turbulence model evaluated on cell centres.
class TurbulenceModel {
 public:
  virtual ~TurbulenceModel() = default;

  /**
   * @brief Advances the turbulence quantities using the current velocity.
   * @param u_faces Face velocities of size (nx+1) x ny.
   * @param v_faces Face velocities of size nx x (ny+1).
   */
  virtual void Update(const Field2D& u_faces, const Field2D& v_faces) = 0;

  /// @brief Eddy viscosity nu_t [m^2/s] at cell centres.
  virtual const Field2D& EddyViscosity() const = 0;

  /// @brief Turbulent kinetic energy k [m^2/s^2] at cell centres.
  virtual const Field2D& TurbulentKineticEnergy() const = 0;

  /// @brief Turbulent dissipation rate epsilon [m^2/s^3] at cell centres.
  virtual const Field2D& Dissipation() const = 0;

  /**
   * @brief Effective viscosity [m^2/s] that reproduces the modelled wall shear
   *        stress for a cell centre at the given distance from a wall.
   */
  virtual double WallViscosity(std::size_t i, std::size_t j,
                               double distance) const = 0;
};

}  // namespace core::flow

#endif  // CORE_FLOW_TURBULENCE_MODEL_HPP_
