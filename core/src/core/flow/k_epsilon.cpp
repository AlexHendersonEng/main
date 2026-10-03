#include "flow/k_epsilon.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace core::flow {

namespace {

constexpr double kMinK = 1e-10;
constexpr double kMinEpsilon = 1e-10;
constexpr double kMaxViscosityRatio = 1e5;

}  // namespace

KEpsilon::KEpsilon(const Grid& grid, const FluidProperties& fluid,
                   const BoundaryConditions& boundaries,
                   const KEpsilonOptions& options)
    : grid_(grid),
      fluid_(fluid),
      boundaries_(boundaries),
      options_(options),
      k_(grid.nx(), grid.ny(), options.initial_k),
      epsilon_(grid.nx(), grid.ny(), options.initial_epsilon),
      nut_(grid.nx(), grid.ny()),
      system_(grid.nx(), grid.ny()) {
  const bool valid_constants =
      options.c_mu > 0.0 && options.c1 > 0.0 && options.c2 > 0.0 &&
      options.sigma_k > 0.0 && options.sigma_epsilon > 0.0 &&
      options.kappa > 0.0 && options.wall_roughness > 0.0 &&
      options.initial_k > 0.0 && options.initial_epsilon > 0.0 &&
      options.sweeps > 0;
  if (!valid_constants) {
    throw std::invalid_argument("k-epsilon options must be positive.");
  }
  if (options.relaxation <= 0.0 || options.relaxation > 1.0) {
    throw std::invalid_argument("k-epsilon relaxation must be in (0, 1].");
  }

  // Start from the inlet turbulence state when an inlet is present
  const BoundaryCondition* inlet = nullptr;
  for (const auto* b : {&boundaries_.west, &boundaries_.east,
                        &boundaries_.south, &boundaries_.north}) {
    if (b->type != BoundaryType::kInlet) {
      continue;
    }
    if (b->k <= 0.0 || b->epsilon <= 0.0) {
      throw std::invalid_argument(
          "Inlet boundaries need positive k and epsilon for k-epsilon.");
    }
    if (inlet == nullptr) {
      inlet = b;
    }
  }
  if (inlet != nullptr) {
    k_.Fill(inlet->k);
    epsilon_.Fill(inlet->epsilon);
  }

  // Solve ln(E y) / kappa = y for the laminar sub-layer limit
  double y = 11.0;
  for (int n = 0; n < 50; ++n) {
    y = std::log(options_.wall_roughness * y) / options_.kappa;
  }
  y_star_laminar_ = y;

  UpdateEddyViscosity();
}

double KEpsilon::WallViscosity(std::size_t i, std::size_t j,
                               double distance) const {
  const double nu = fluid_.kinematic_viscosity;
  const double y_star = std::pow(options_.c_mu, 0.25) *
                        std::sqrt(std::max(k_(i, j), kMinK)) * distance / nu;
  if (y_star <= y_star_laminar_) {
    return nu;
  }
  return nu * y_star * options_.kappa /
         std::log(options_.wall_roughness * y_star);
}

KEpsilon::WallInfo KEpsilon::FindWall(std::size_t i, std::size_t j,
                                      const Field2D& cell_u,
                                      const Field2D& cell_v) const {
  const std::size_t nx = grid_.nx();
  const std::size_t ny = grid_.ny();
  WallInfo best;

  auto consider = [&best](double distance, double speed) {
    if (!best.present || distance < best.distance) {
      best = {true, distance, speed};
    }
  };

  const double hy = 0.5 * grid_.dy();
  const double hx = 0.5 * grid_.dx();
  const auto& bc = boundaries_;

  if (j == 0 ? bc.south.type == BoundaryType::kWall : grid_.IsSolid(i, j - 1)) {
    consider(hy, std::abs(cell_u(i, j) - (j == 0 ? bc.south.u : 0.0)));
  }
  if (j + 1 == ny ? bc.north.type == BoundaryType::kWall
                  : grid_.IsSolid(i, j + 1)) {
    consider(hy, std::abs(cell_u(i, j) - (j + 1 == ny ? bc.north.u : 0.0)));
  }
  if (i == 0 ? bc.west.type == BoundaryType::kWall : grid_.IsSolid(i - 1, j)) {
    consider(hx, std::abs(cell_v(i, j) - (i == 0 ? bc.west.v : 0.0)));
  }
  if (i + 1 == nx ? bc.east.type == BoundaryType::kWall
                  : grid_.IsSolid(i + 1, j)) {
    consider(hx, std::abs(cell_v(i, j) - (i + 1 == nx ? bc.east.v : 0.0)));
  }
  return best;
}

void KEpsilon::SolveTransport(Field2D& phi, const Field2D& source,
                              const Field2D& sink, const double sigma,
                              double BoundaryCondition::* inlet_value,
                              const Field2D& u_faces, const Field2D& v_faces,
                              const Field2D& fixed) {
  const std::size_t nx = grid_.nx();
  const std::size_t ny = grid_.ny();
  const double dx = grid_.dx();
  const double dy = grid_.dy();
  const double nu = fluid_.kinematic_viscosity;
  const double alpha = options_.relaxation;
  const double volume = dx * dy;

  auto gamma = [&](std::size_t i, std::size_t j) {
    return nu + nut_(i, j) / sigma;
  };

  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      system_.c(i, j) = system_.e(i, j) = system_.w(i, j) = 0.0;
      system_.n(i, j) = system_.s(i, j) = system_.b(i, j) = 0.0;
      if (grid_.IsSolid(i, j) || fixed(i, j) > 0.0) {
        continue;
      }

      double ap = 0.0;
      double b = 0.0;

      // Adds one cell face using upwind convection and central diffusion
      auto add_face = [&](const bool interior, const std::size_t ni,
                          const std::size_t nj, const double outward_flux,
                          const double area, const double spacing,
                          const BoundaryCondition& bc, Field2D& coefficient) {
        if (interior) {
          if (grid_.IsSolid(ni, nj)) {
            return;
          }
          const double d = 0.5 * (gamma(i, j) + gamma(ni, nj)) * area / spacing;
          coefficient(i, j) = d + std::max(-outward_flux, 0.0);
          ap += d + std::max(outward_flux, 0.0);
        } else if (bc.type == BoundaryType::kOutlet) {
          ap += std::max(outward_flux, 0.0);
          b += std::max(-outward_flux, 0.0) * phi(i, j);
        } else if (bc.type == BoundaryType::kInlet) {
          const double d = gamma(i, j) * area / (0.5 * spacing);
          ap += d + std::max(outward_flux, 0.0);
          b += (d + std::max(-outward_flux, 0.0)) * (bc.*inlet_value);
        }
      };

      add_face(i + 1 < nx, i + 1, j, u_faces(i + 1, j) * dy, dy, dx,
               boundaries_.east, system_.e);
      add_face(i > 0, i - 1, j, -u_faces(i, j) * dy, dy, dx, boundaries_.west,
               system_.w);
      add_face(j + 1 < ny, i, j + 1, v_faces(i, j + 1) * dx, dx, dy,
               boundaries_.north, system_.n);
      add_face(j > 0, i, j - 1, -v_faces(i, j) * dx, dx, dy, boundaries_.south,
               system_.s);

      ap += sink(i, j) * volume;
      b += source(i, j) * volume;
      ap /= alpha;
      b += (1.0 - alpha) * ap * phi(i, j);

      system_.c(i, j) = ap;
      system_.b(i, j) = b;
    }
  }

  LinearSolveOptions solve;
  solve.max_iterations = options_.sweeps;
  solve.relative_tolerance = 1e-3;
  SolveSor(system_, phi, solve);
}

void KEpsilon::UpdateEddyViscosity() {
  const double nu = fluid_.kinematic_viscosity;
  for (std::size_t j = 0; j < grid_.ny(); ++j) {
    for (std::size_t i = 0; i < grid_.nx(); ++i) {
      if (grid_.IsSolid(i, j)) {
        nut_(i, j) = 0.0;
        continue;
      }
      const double k = std::max(k_(i, j), kMinK);
      const double eps = std::max(epsilon_(i, j), kMinEpsilon);
      nut_(i, j) =
          std::min(options_.c_mu * k * k / eps, kMaxViscosityRatio * nu);
    }
  }
}

void KEpsilon::Update(const Field2D& u_faces, const Field2D& v_faces) {
  const std::size_t nx = grid_.nx();
  const std::size_t ny = grid_.ny();
  const double dx = grid_.dx();
  const double dy = grid_.dy();
  const double c_mu_quarter = std::pow(options_.c_mu, 0.25);
  const double c_mu_three_quarters = std::pow(options_.c_mu, 0.75);

  Field2D cell_u(nx, ny);
  Field2D cell_v(nx, ny);
  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      cell_u(i, j) = 0.5 * (u_faces(i, j) + u_faces(i + 1, j));
      cell_v(i, j) = 0.5 * (v_faces(i, j) + v_faces(i, j + 1));
    }
  }

  Field2D wall(nx, ny);
  Field2D production(nx, ny);
  Field2D source(nx, ny);
  Field2D sink(nx, ny);

  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      if (grid_.IsSolid(i, j)) {
        continue;
      }
      const WallInfo info = FindWall(i, j, cell_u, cell_v);
      const double k = std::max(k_(i, j), kMinK);

      if (info.present) {
        // Wall function: production from the modelled wall shear stress
        wall(i, j) = 1.0;
        const double shear =
            WallViscosity(i, j, info.distance) * info.speed / info.distance;
        production(i, j) = shear * c_mu_quarter * std::sqrt(k) /
                           (options_.kappa * info.distance);
      } else {
        // Strain rate from face velocities and central differences
        const std::size_t js = j > 0 ? j - 1 : j;
        const std::size_t jn = j + 1 < ny ? j + 1 : j;
        const std::size_t iw = i > 0 ? i - 1 : i;
        const std::size_t ie = i + 1 < nx ? i + 1 : i;
        const double ux = (u_faces(i + 1, j) - u_faces(i, j)) / dx;
        const double vy = (v_faces(i, j + 1) - v_faces(i, j)) / dy;
        const double uy = jn > js ? (cell_u(i, jn) - cell_u(i, js)) /
                                        (static_cast<double>(jn - js) * dy)
                                  : 0.0;
        const double vx = ie > iw ? (cell_v(ie, j) - cell_v(iw, j)) /
                                        (static_cast<double>(ie - iw) * dx)
                                  : 0.0;
        const double strain_squared =
            2.0 * (ux * ux + vy * vy) + (uy + vx) * (uy + vx);
        production(i, j) = nut_(i, j) * strain_squared;
      }
    }
  }

  // Wall cells use the equilibrium dissipation, evaluated for the current k
  auto fix_wall_dissipation = [&]() {
    for (std::size_t j = 0; j < ny; ++j) {
      for (std::size_t i = 0; i < nx; ++i) {
        if (wall(i, j) > 0.0) {
          const WallInfo info = FindWall(i, j, cell_u, cell_v);
          epsilon_(i, j) = c_mu_three_quarters *
                           std::pow(std::max(k_(i, j), kMinK), 1.5) /
                           (options_.kappa * info.distance);
        }
      }
    }
  };
  fix_wall_dissipation();

  // Turbulent kinetic energy
  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      source(i, j) = production(i, j);
      sink(i, j) =
          std::max(epsilon_(i, j), kMinEpsilon) / std::max(k_(i, j), kMinK);
    }
  }
  SolveTransport(k_, source, sink, options_.sigma_k, &BoundaryCondition::k,
                 u_faces, v_faces, Field2D(nx, ny));
  auto clamp_fields = [&]() {
    for (std::size_t j = 0; j < ny; ++j) {
      for (std::size_t i = 0; i < nx; ++i) {
        k_(i, j) = std::max(k_(i, j), kMinK);
        epsilon_(i, j) = std::max(epsilon_(i, j), kMinEpsilon);
      }
    }
  };
  clamp_fields();
  fix_wall_dissipation();

  // Dissipation rate, fixed in wall cells
  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      const double time_scale_inverse = epsilon_(i, j) / k_(i, j);
      source(i, j) = options_.c1 * time_scale_inverse * production(i, j);
      sink(i, j) = options_.c2 * time_scale_inverse;
    }
  }
  SolveTransport(epsilon_, source, sink, options_.sigma_epsilon,
                 &BoundaryCondition::epsilon, u_faces, v_faces, wall);
  clamp_fields();

  UpdateEddyViscosity();
}

}  // namespace core::flow
