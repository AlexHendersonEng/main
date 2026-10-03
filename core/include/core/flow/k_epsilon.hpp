#ifndef CORE_FLOW_K_EPSILON_HPP_
#define CORE_FLOW_K_EPSILON_HPP_

/**
 * @file k_epsilon.hpp
 * @brief Standard k-epsilon turbulence model with standard wall functions.
 *
 * The eddy viscosity is
 * \f[ \nu_t = C_\mu \frac{k^2}{\epsilon} \f]
 * and k and epsilon are advanced with finite-volume transport equations on
 * the cell centres, using the staggered face velocities as convective fluxes
 * with first-order upwinding.
 *
 * Cells next to a wall (domain wall or solid cell) use wall functions: the
 * dissipation is fixed to \f$C_\mu^{3/4} k^{3/2} / (\kappa y)\f$ and the
 * production of k is derived from the modelled wall shear stress.
 */

#include <cstddef>

#include "flow/boundary_conditions.hpp"
#include "flow/field.hpp"
#include "flow/fluid_properties.hpp"
#include "flow/grid.hpp"
#include "flow/stencil_solver.hpp"
#include "flow/turbulence_model.hpp"

namespace core::flow {

struct KEpsilonOptions {
  double c_mu = 0.09;
  double c1 = 1.44;
  double c2 = 1.92;
  double sigma_k = 1.0;
  double sigma_epsilon = 1.3;
  double kappa = 0.41;            ///< Von Karman constant.
  double wall_roughness = 9.793;  ///< Log-law constant E for smooth walls.
  double relaxation = 0.7;        ///< Under-relaxation for k and epsilon.
  int sweeps = 10;          ///< Linear solver sweeps per transport equation.
  double initial_k = 1e-3;  ///< Used where no inlet value is available.
  double initial_epsilon = 1e-3;  ///< Used where no inlet value is available.
};

class KEpsilon : public TurbulenceModel {
 public:
  /**
   * @throws std::invalid_argument if an option is not positive, relaxation is
   *         not in (0, 1], or an inlet has non-positive k or epsilon.
   */
  KEpsilon(const Grid& grid, const FluidProperties& fluid,
           const BoundaryConditions& boundaries,
           const KEpsilonOptions& options = {});

  void Update(const Field2D& u_faces, const Field2D& v_faces) override;

  const Field2D& EddyViscosity() const override { return nut_; }
  const Field2D& TurbulentKineticEnergy() const override { return k_; }
  const Field2D& Dissipation() const override { return epsilon_; }

  double WallViscosity(std::size_t i, std::size_t j,
                       double distance) const override;

 private:
  struct WallInfo {
    bool present = false;
    double distance = 0.0;
    double speed = 0.0;  ///< Fluid speed relative to the wall.
  };

  WallInfo FindWall(std::size_t i, std::size_t j, const Field2D& cell_u,
                    const Field2D& cell_v) const;
  void SolveTransport(Field2D& phi, const Field2D& source, const Field2D& sink,
                      double sigma, double BoundaryCondition::* inlet_value,
                      const Field2D& u_faces, const Field2D& v_faces,
                      const Field2D& fixed);
  void UpdateEddyViscosity();

  Grid grid_;
  FluidProperties fluid_;
  BoundaryConditions boundaries_;
  KEpsilonOptions options_;
  double y_star_laminar_ = 11.225;  ///< Intersection of linear and log laws.
  Field2D k_, epsilon_, nut_;
  StencilSystem system_;
};

}  // namespace core::flow

#endif  // CORE_FLOW_K_EPSILON_HPP_
