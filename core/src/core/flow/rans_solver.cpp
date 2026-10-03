#include "flow/rans_solver.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace core::flow {

namespace {

bool IsDirichlet(BoundaryType t) {
  return t == BoundaryType::kWall || t == BoundaryType::kInlet;
}

double Pos(double x) { return std::max(x, 0.0); }

}  // namespace

RansSolver::RansSolver(const Grid& grid, const RansSolverConfig& config)
    : grid_(grid),
      config_(config),
      u_(grid.nx() + 1, grid.ny()),
      v_(grid.nx(), grid.ny() + 1),
      p_(grid.nx(), grid.ny()),
      du_(grid.nx() + 1, grid.ny()),
      dv_(grid.nx(), grid.ny() + 1),
      fixed_u_(grid.nx() + 1, grid.ny()),
      fixed_v_(grid.nx(), grid.ny() + 1),
      outlet_u_(grid.nx() + 1, grid.ny()),
      outlet_v_(grid.nx(), grid.ny() + 1),
      sys_u_(grid.nx() + 1, grid.ny()),
      sys_v_(grid.nx(), grid.ny() + 1),
      sys_p_(grid.nx(), grid.ny()),
      nu_eff_(grid.nx(), grid.ny(), config.fluid.kinematic_viscosity) {
  auto in_range = [](double a) { return a > 0.0 && a <= 1.0; };
  if (!in_range(config.velocity_relaxation) ||
      !in_range(config.pressure_relaxation)) {
    throw std::invalid_argument("Relaxation factors must be in (0, 1]");
  }
  if (config.turbulence_model == TurbulenceModelType::kKEpsilon) {
    turbulence_ = std::make_unique<KEpsilon>(
        grid_, config_.fluid, config_.boundaries, config_.k_epsilon);
  }

  const std::size_t nx = grid_.nx();
  const std::size_t ny = grid_.ny();
  const auto& bc = config_.boundaries;

  auto note_outlet = [&](const BoundaryCondition& b) {
    if (b.type == BoundaryType::kOutlet && !has_outlet_) {
      has_outlet_ = true;
      outlet_pressure_ = b.pressure;
    }
  };
  note_outlet(bc.west);
  note_outlet(bc.east);
  note_outlet(bc.south);
  note_outlet(bc.north);

  // u faces: fixed by solid neighbours, wall/inlet/symmetry boundaries.
  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i <= nx; ++i) {
      const bool solid_w = i > 0 && grid_.IsSolid(i - 1, j);
      const bool solid_e = i < nx && grid_.IsSolid(i, j);
      if (solid_w || solid_e) {
        fixed_u_(i, j) = 1.0;
      } else if (i == 0 || i == nx) {
        const auto& b = (i == 0) ? bc.west : bc.east;
        fixed_u_(i, j) = 1.0;
        if (b.type == BoundaryType::kOutlet) {
          outlet_u_(i, j) = 1.0;
        } else if (b.type == BoundaryType::kInlet) {
          u_(i, j) = b.u;
        }
      }
    }
  }
  for (std::size_t j = 0; j <= ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      const bool solid_s = j > 0 && grid_.IsSolid(i, j - 1);
      const bool solid_n = j < ny && grid_.IsSolid(i, j);
      if (solid_s || solid_n) {
        fixed_v_(i, j) = 1.0;
      } else if (j == 0 || j == ny) {
        const auto& b = (j == 0) ? bc.south : bc.north;
        fixed_v_(i, j) = 1.0;
        if (b.type == BoundaryType::kOutlet) {
          outlet_v_(i, j) = 1.0;
        } else if (b.type == BoundaryType::kInlet) {
          v_(i, j) = b.v;
        }
      }
    }
  }

  double ref = 0.0;
  for (const auto* b : {&bc.west, &bc.east, &bc.south, &bc.north}) {
    ref = std::max({ref, std::abs(b->u), std::abs(b->v)});
  }
  u_ref_ = ref > 0.0 ? ref : 1.0;
}

double RansSolver::WallViscosity(const std::size_t ia, const std::size_t ja,
                                 const std::size_t ib, const std::size_t jb,
                                 const double distance) const {
  if (!turbulence_) {
    return config_.fluid.kinematic_viscosity;
  }
  return 0.5 * (turbulence_->WallViscosity(ia, ja, distance) +
                turbulence_->WallViscosity(ib, jb, distance));
}

void RansSolver::AssembleU() {
  const std::size_t nx = grid_.nx();
  const std::size_t ny = grid_.ny();
  const double dx = grid_.dx();
  const double dy = grid_.dy();
  const double alpha = config_.velocity_relaxation;
  const auto& bc = config_.boundaries;

  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i <= nx; ++i) {
      sys_u_.c(i, j) = sys_u_.e(i, j) = sys_u_.w(i, j) = 0.0;
      sys_u_.n(i, j) = sys_u_.s(i, j) = sys_u_.b(i, j) = 0.0;
      du_(i, j) = 0.0;
      if (fixed_u_(i, j) > 0.0) continue;

      const double fe = 0.5 * dy * (u_(i, j) + u_(i + 1, j));
      const double fw = 0.5 * dy * (u_(i - 1, j) + u_(i, j));
      const double fn = 0.5 * dx * (v_(i - 1, j + 1) + v_(i, j + 1));
      const double fs = 0.5 * dx * (v_(i - 1, j) + v_(i, j));

      const double ae = nu_eff_(i, j) * dy / dx + Pos(-fe);
      const double aw = nu_eff_(i - 1, j) * dy / dx + Pos(fw);
      double an = 0.0;
      double as = 0.0;
      double b = 0.0;

      // Wall/inlet boundaries and solid surfaces lie half a cell away.
      if (j + 1 == ny) {
        if (IsDirichlet(bc.north.type)) {
          const double nu_w = bc.north.type == BoundaryType::kWall
                                  ? WallViscosity(i - 1, j, i, j, 0.5 * dy)
                                  : 0.5 * (nu_eff_(i - 1, j) + nu_eff_(i, j));
          an = 2.0 * nu_w * dx / dy;
          b += an * bc.north.u;
        }
      } else if (grid_.IsSolid(i - 1, j + 1) && grid_.IsSolid(i, j + 1)) {
        an = 2.0 * WallViscosity(i - 1, j, i, j, 0.5 * dy) * dx / dy;
      } else {
        const double nu_n = 0.25 * (nu_eff_(i - 1, j) + nu_eff_(i, j) +
                                    nu_eff_(i - 1, j + 1) + nu_eff_(i, j + 1));
        an = nu_n * dx / dy + Pos(-fn);
      }
      if (j == 0) {
        if (IsDirichlet(bc.south.type)) {
          const double nu_w = bc.south.type == BoundaryType::kWall
                                  ? WallViscosity(i - 1, j, i, j, 0.5 * dy)
                                  : 0.5 * (nu_eff_(i - 1, j) + nu_eff_(i, j));
          as = 2.0 * nu_w * dx / dy;
          b += as * bc.south.u;
        }
      } else if (grid_.IsSolid(i - 1, j - 1) && grid_.IsSolid(i, j - 1)) {
        as = 2.0 * WallViscosity(i - 1, j, i, j, 0.5 * dy) * dx / dy;
      } else {
        const double nu_s = 0.25 * (nu_eff_(i - 1, j) + nu_eff_(i, j) +
                                    nu_eff_(i - 1, j - 1) + nu_eff_(i, j - 1));
        as = nu_s * dx / dy + Pos(fs);
      }

      const double ap = (ae + aw + an + as) / alpha;
      b += (p_(i - 1, j) - p_(i, j)) * dy + (1.0 - alpha) * ap * u_(i, j);

      sys_u_.c(i, j) = ap;
      sys_u_.e(i, j) = ae;
      sys_u_.w(i, j) = aw;
      sys_u_.n(i, j) = an;
      sys_u_.s(i, j) = as;
      sys_u_.b(i, j) = b;
      du_(i, j) = dy / ap;
    }
  }
}

void RansSolver::AssembleV() {
  const std::size_t nx = grid_.nx();
  const std::size_t ny = grid_.ny();
  const double dx = grid_.dx();
  const double dy = grid_.dy();
  const double alpha = config_.velocity_relaxation;
  const auto& bc = config_.boundaries;

  for (std::size_t j = 0; j <= ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      sys_v_.c(i, j) = sys_v_.e(i, j) = sys_v_.w(i, j) = 0.0;
      sys_v_.n(i, j) = sys_v_.s(i, j) = sys_v_.b(i, j) = 0.0;
      dv_(i, j) = 0.0;
      if (fixed_v_(i, j) > 0.0) continue;

      const double fe = 0.5 * dy * (u_(i + 1, j - 1) + u_(i + 1, j));
      const double fw = 0.5 * dy * (u_(i, j - 1) + u_(i, j));
      const double fn = 0.5 * dx * (v_(i, j) + v_(i, j + 1));
      const double fs = 0.5 * dx * (v_(i, j - 1) + v_(i, j));

      const double an = nu_eff_(i, j) * dx / dy + Pos(-fn);
      const double as = nu_eff_(i, j - 1) * dx / dy + Pos(fs);
      double ae = 0.0;
      double aw = 0.0;
      double b = 0.0;

      if (i + 1 == nx) {
        if (IsDirichlet(bc.east.type)) {
          const double nu_w = bc.east.type == BoundaryType::kWall
                                  ? WallViscosity(i, j - 1, i, j, 0.5 * dx)
                                  : 0.5 * (nu_eff_(i, j - 1) + nu_eff_(i, j));
          ae = 2.0 * nu_w * dy / dx;
          b += ae * bc.east.v;
        }
      } else if (grid_.IsSolid(i + 1, j - 1) && grid_.IsSolid(i + 1, j)) {
        ae = 2.0 * WallViscosity(i, j - 1, i, j, 0.5 * dx) * dy / dx;
      } else {
        const double nu_e = 0.25 * (nu_eff_(i, j - 1) + nu_eff_(i, j) +
                                    nu_eff_(i + 1, j - 1) + nu_eff_(i + 1, j));
        ae = nu_e * dy / dx + Pos(-fe);
      }
      if (i == 0) {
        if (IsDirichlet(bc.west.type)) {
          const double nu_w = bc.west.type == BoundaryType::kWall
                                  ? WallViscosity(i, j - 1, i, j, 0.5 * dx)
                                  : 0.5 * (nu_eff_(i, j - 1) + nu_eff_(i, j));
          aw = 2.0 * nu_w * dy / dx;
          b += aw * bc.west.v;
        }
      } else if (grid_.IsSolid(i - 1, j - 1) && grid_.IsSolid(i - 1, j)) {
        aw = 2.0 * WallViscosity(i, j - 1, i, j, 0.5 * dx) * dy / dx;
      } else {
        const double nu_w = 0.25 * (nu_eff_(i, j - 1) + nu_eff_(i, j) +
                                    nu_eff_(i - 1, j - 1) + nu_eff_(i - 1, j));
        aw = nu_w * dy / dx + Pos(fw);
      }

      const double ap = (ae + aw + an + as) / alpha;
      b += (p_(i, j - 1) - p_(i, j)) * dx + (1.0 - alpha) * ap * v_(i, j);

      sys_v_.c(i, j) = ap;
      sys_v_.e(i, j) = ae;
      sys_v_.w(i, j) = aw;
      sys_v_.n(i, j) = an;
      sys_v_.s(i, j) = as;
      sys_v_.b(i, j) = b;
      dv_(i, j) = dx / ap;
    }
  }
}
void RansSolver::ApplyOutletCopy() {
  const std::size_t nx = grid_.nx();
  const std::size_t ny = grid_.ny();
  for (std::size_t j = 0; j < ny; ++j) {
    if (outlet_u_(0, j) > 0.0 && nx > 1) {
      u_(0, j) = u_(1, j);
      du_(0, j) = du_(1, j);
    }
    if (outlet_u_(nx, j) > 0.0 && nx > 1) {
      u_(nx, j) = u_(nx - 1, j);
      du_(nx, j) = du_(nx - 1, j);
    }
  }
  for (std::size_t i = 0; i < nx; ++i) {
    if (outlet_v_(i, 0) > 0.0 && ny > 1) {
      v_(i, 0) = v_(i, 1);
      dv_(i, 0) = dv_(i, 1);
    }
    if (outlet_v_(i, ny) > 0.0 && ny > 1) {
      v_(i, ny) = v_(i, ny - 1);
      dv_(i, ny) = dv_(i, ny - 1);
    }
  }
}

// Builds and solves the pressure-correction equation; returns the max cell
// mass imbalance before correction. Outlet ghost pressure correction is zero.
double RansSolver::SolvePressureCorrection(Field2D& pc) {
  const std::size_t nx = grid_.nx();
  const std::size_t ny = grid_.ny();
  const double dx = grid_.dx();
  const double dy = grid_.dy();
  double max_imbalance = 0.0;

  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      sys_p_.c(i, j) = sys_p_.e(i, j) = sys_p_.w(i, j) = 0.0;
      sys_p_.n(i, j) = sys_p_.s(i, j) = sys_p_.b(i, j) = 0.0;
      if (grid_.IsSolid(i, j)) continue;

      const double ce = dy * du_(i + 1, j);
      const double cw = dy * du_(i, j);
      const double cn = dx * dv_(i, j + 1);
      const double cs = dx * dv_(i, j);
      const double c = ce + cw + cn + cs;
      if (c <= 0.0) continue;

      sys_p_.c(i, j) = c;
      sys_p_.e(i, j) = (i + 1 < nx) ? ce : 0.0;
      sys_p_.w(i, j) = (i > 0) ? cw : 0.0;
      sys_p_.n(i, j) = (j + 1 < ny) ? cn : 0.0;
      sys_p_.s(i, j) = (j > 0) ? cs : 0.0;
      const double b =
          (u_(i, j) - u_(i + 1, j)) * dy + (v_(i, j) - v_(i, j + 1)) * dx;
      sys_p_.b(i, j) = b;
      max_imbalance = std::max(max_imbalance, std::abs(b));
    }
  }

  pc.Fill(0.0);
  LinearSolveOptions options;
  options.omega = 1.7;
  options.max_iterations = config_.pressure_sweeps;
  options.relative_tolerance = 1e-3;
  SolveSor(sys_p_, pc, options);
  return max_imbalance;
}

SolveResult RansSolver::Step() {
  const std::size_t nx = grid_.nx();
  const std::size_t ny = grid_.ny();
  const double alpha_p = config_.pressure_relaxation;

  const Field2D u_old = u_;
  const Field2D v_old = v_;

  LinearSolveOptions momentum;
  momentum.max_iterations = config_.momentum_sweeps;
  momentum.relative_tolerance = 1e-3;

  if (turbulence_) {
    turbulence_->Update(u_, v_);
    for (std::size_t j = 0; j < ny; ++j) {
      for (std::size_t i = 0; i < nx; ++i) {
        nu_eff_(i, j) = config_.fluid.kinematic_viscosity +
                        turbulence_->EddyViscosity()(i, j);
      }
    }
  }

  AssembleU();
  SolveSor(sys_u_, u_, momentum);
  AssembleV();
  SolveSor(sys_v_, v_, momentum);
  ApplyOutletCopy();

  Field2D pc(nx, ny);
  const double imbalance = SolvePressureCorrection(pc);

  auto pcorr = [&](std::size_t i, std::size_t j, bool inside) {
    return inside ? pc(i, j) : 0.0;
  };

  // Correct face velocities; fixed faces have zero d and stay unchanged.
  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i <= nx; ++i) {
      const double pw = pcorr(i - 1, j, i > 0);
      const double pe = pcorr(i, j, i < nx);
      u_(i, j) += du_(i, j) * (pw - pe);
    }
  }
  for (std::size_t j = 0; j <= ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      const double ps = pcorr(i, j - 1, j > 0);
      const double pn = pcorr(i, j, j < ny);
      v_(i, j) += dv_(i, j) * (ps - pn);
    }
  }

  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      if (!grid_.IsSolid(i, j)) p_(i, j) += alpha_p * pc(i, j);
    }
  }

  // Without an outlet the pressure level is arbitrary; remove the mean.
  if (!has_outlet_) {
    double sum = 0.0;
    std::size_t count = 0;
    for (std::size_t j = 0; j < ny; ++j) {
      for (std::size_t i = 0; i < nx; ++i) {
        if (!grid_.IsSolid(i, j)) {
          sum += p_(i, j);
          ++count;
        }
      }
    }
    const double mean = count ? sum / static_cast<double>(count) : 0.0;
    for (std::size_t j = 0; j < ny; ++j) {
      for (std::size_t i = 0; i < nx; ++i) {
        if (!grid_.IsSolid(i, j)) p_(i, j) -= mean;
      }
    }
  }

  double change = 0.0;
  for (std::size_t n = 0; n < u_.size(); ++n) {
    change = std::max(change, std::abs(u_.data()[n] - u_old.data()[n]));
  }
  for (std::size_t n = 0; n < v_.size(); ++n) {
    change = std::max(change, std::abs(v_.data()[n] - v_old.data()[n]));
  }

  SolveResult result;
  result.mass_residual =
      imbalance / (u_ref_ * std::max(grid_.dx(), grid_.dy()));
  result.velocity_change = change / u_ref_;
  history_.push_back(result.mass_residual);
  return result;
}

SolveResult RansSolver::Solve() {
  SolveResult result;
  for (int it = 1; it <= config_.max_iterations; ++it) {
    result = Step();
    result.iterations = it;
    if (result.mass_residual < config_.tolerance &&
        result.velocity_change < config_.tolerance) {
      result.converged = true;
      break;
    }
  }
  return result;
}

Field2D RansSolver::Pressure() const {
  Field2D p(grid_.nx(), grid_.ny());
  const double rho = config_.fluid.density;
  const double offset = has_outlet_ ? outlet_pressure_ : 0.0;
  for (std::size_t j = 0; j < grid_.ny(); ++j) {
    for (std::size_t i = 0; i < grid_.nx(); ++i) {
      if (!grid_.IsSolid(i, j)) p(i, j) = rho * p_(i, j) + offset;
    }
  }
  return p;
}

Field2D RansSolver::EddyViscosity() const {
  return turbulence_ ? turbulence_->EddyViscosity()
                     : Field2D(grid_.nx(), grid_.ny());
}

Field2D RansSolver::TurbulentKineticEnergy() const {
  return turbulence_ ? turbulence_->TurbulentKineticEnergy()
                     : Field2D(grid_.nx(), grid_.ny());
}

Field2D RansSolver::Dissipation() const {
  return turbulence_ ? turbulence_->Dissipation()
                     : Field2D(grid_.nx(), grid_.ny());
}

Field2D RansSolver::CellU() const {
  Field2D f(grid_.nx(), grid_.ny());
  for (std::size_t j = 0; j < grid_.ny(); ++j) {
    for (std::size_t i = 0; i < grid_.nx(); ++i) {
      f(i, j) = 0.5 * (u_(i, j) + u_(i + 1, j));
    }
  }
  return f;
}

Field2D RansSolver::CellV() const {
  Field2D f(grid_.nx(), grid_.ny());
  for (std::size_t j = 0; j < grid_.ny(); ++j) {
    for (std::size_t i = 0; i < grid_.nx(); ++i) {
      f(i, j) = 0.5 * (v_(i, j) + v_(i, j + 1));
    }
  }
  return f;
}

}  // namespace core::flow
