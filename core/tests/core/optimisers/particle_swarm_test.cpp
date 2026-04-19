#include "optimisers/particle_swarm.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <functional>
#include <limits>
#include <vector>

#include "utils/random.hpp"

namespace {

double Quadratic(const std::vector<double>& x) {
  // f(x) = (x[0]-3)^2 + (x[1]+2)^2
  return std::pow(x[0] - 3.0, 2) + std::pow(x[1] + 2.0, 2);
}

}  // namespace

class ParticleSwarmTest : public ::testing::Test {
 protected:
  void SetUp() override { vanta::utils::SetRandomSeed(42); }
};

TEST_F(ParticleSwarmTest, RespectsBounds) {
  // Set bounds
  std::vector<double> lb = {-5.0, -5.0};
  std::vector<double> ub = {5.0, 5.0};

  // Optimiser options
  vanta::optimisers::PSOptions opts;
  opts.n_particles = 30;
  opts.max_iters = 50;
  opts.tolerance = 1e-6;

  // Solve
  auto sol = ParticleSwarm(Quadratic, lb, ub, opts);

  // Check bounds
  for (size_t i = 0; i < sol.x.size(); ++i) {
    EXPECT_GE(sol.x[i], lb[i]);
    EXPECT_LE(sol.x[i], ub[i]);
  }
}

TEST_F(ParticleSwarmTest, ImprovesObjectiveValue) {
  // Set bounds
  std::vector<double> lb = {-10.0, -10.0};
  std::vector<double> ub = {10.0, 10.0};

  // Optimiser options
  vanta::optimisers::PSOptions opts;
  opts.n_particles = 40;
  opts.max_iters = 80;
  opts.tolerance = 1e-6;

  // Solve
  auto sol = ParticleSwarm(Quadratic, lb, ub, opts);

  // Check solution
  EXPECT_LT(sol.f_val, 1.0);
}

TEST_F(ParticleSwarmTest, ConvergenceFlagMatchesTolerance) {
  // Set bounds
  std::vector<double> lb = {-5.0, -5.0};
  std::vector<double> ub = {5.0, 5.0};

  // Optimiser options
  vanta::optimisers::PSOptions opts;
  opts.n_particles = 50;
  opts.max_iters = 100;
  opts.tolerance = 1e-3;

  // Solve
  auto sol = ParticleSwarm(Quadratic, lb, ub, opts);

  // Check convergence flag
  EXPECT_EQ(sol.converged, sol.f_val < opts.tolerance);
}

TEST_F(ParticleSwarmTest, HandlesSmallSwarm) {
  // Set bounds
  std::vector<double> lb = {-5.0, -5.0};
  std::vector<double> ub = {5.0, 5.0};

  // Optimiser options
  vanta::optimisers::PSOptions opts;
  opts.n_particles = 1;
  opts.max_iters = 20;
  opts.tolerance = 1e-6;

  // Solve
  auto sol = ParticleSwarm(Quadratic, lb, ub, opts);

  // Check solution is finite and has correct dimensionality
  EXPECT_TRUE(std::isfinite(sol.f_val));
  EXPECT_EQ(sol.x.size(), 2);
}

TEST_F(ParticleSwarmTest, DimensionConsistency) {
  // Set bounds
  std::vector<double> lb = {-3.0, -3.0, -3.0};
  std::vector<double> ub = {3.0, 3.0, 3.0};

  // Optimiser options
  vanta::optimisers::PSOptions opts;
  opts.n_particles = 20;
  opts.max_iters = 50;
  opts.tolerance = 1e-6;

  // Solve
  auto sol = ParticleSwarm(
      [](const std::vector<double>& x) {
        return x[0] * x[0] + x[1] * x[1] + x[2] * x[2];
      },
      lb, ub, opts);

  // Check solution has correct dimensionality
  EXPECT_EQ(sol.x.size(), 3);
}

TEST_F(ParticleSwarmTest, ApproachesKnownOptimum) {
  // Set bounds
  std::vector<double> lb = {-10.0, -10.0};
  std::vector<double> ub = {10.0, 10.0};

  // Optimiser options
  vanta::optimisers::PSOptions opts;
  opts.n_particles = 60;
  opts.max_iters = 150;
  opts.tolerance = 1e-6;

  // Solve
  auto sol = ParticleSwarm(Quadratic, lb, ub, opts);

  // Check solution is close to known optimum (3, -2)
  EXPECT_NEAR(sol.x[0], 3.0, 1e-3);
  EXPECT_NEAR(sol.x[1], -2.0, 1e-3);
}

TEST_F(ParticleSwarmTest, IterationLimitRespected) {
  // Set bounds
  std::vector<double> lb = {-5.0, -5.0};
  std::vector<double> ub = {5.0, 5.0};

  // Optimiser options
  vanta::optimisers::PSOptions opts;
  opts.n_particles = 30;
  opts.max_iters = 10;
  opts.tolerance = 1e-12;

  // Solve
  auto sol = ParticleSwarm(Quadratic, lb, ub, opts);

  // Check iteration limit is respected
  EXPECT_LE(sol.iters, opts.max_iters);
}

TEST_F(ParticleSwarmTest, ObjectiveIsFinite) {
  // Set bounds
  std::vector<double> lb = {-100.0, -100.0};
  std::vector<double> ub = {100.0, 100.0};

  // Optimiser options
  vanta::optimisers::PSOptions opts;
  opts.n_particles = 25;
  opts.max_iters = 50;
  opts.tolerance = 1e-6;

  // Solve
  auto sol = ParticleSwarm(Quadratic, lb, ub, opts);

  // Check objective function value is finite
  EXPECT_TRUE(std::isfinite(sol.f_val));
}
