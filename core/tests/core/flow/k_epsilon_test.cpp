#include "flow/k_epsilon.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

class KEpsilonTest : public ::testing::Test {
 protected:
  // Allowed numerical tolerance for value comparisons
  const double kTolerance = 1e-9;

  core::flow::Grid grid_{4, 4, 1.0, 1.0};
  core::flow::FluidProperties fluid_{1.0, 1e-5};
  core::flow::BoundaryConditions boundaries_;
};

TEST_F(KEpsilonTest, InvalidRelaxation) {
  core::flow::KEpsilonOptions options;
  options.relaxation = 0.0;

  EXPECT_THROW(
      { core::flow::KEpsilon model(grid_, fluid_, boundaries_, options); },
      std::invalid_argument);
}

TEST_F(KEpsilonTest, InvalidConstant) {
  core::flow::KEpsilonOptions options;
  options.c_mu = 0.0;

  EXPECT_THROW(
      { core::flow::KEpsilon model(grid_, fluid_, boundaries_, options); },
      std::invalid_argument);
}

TEST_F(KEpsilonTest, InletWithoutTurbulenceQuantities) {
  boundaries_.west.type = core::flow::BoundaryType::kInlet;
  boundaries_.west.u = 1.0;

  EXPECT_THROW(
      { core::flow::KEpsilon model(grid_, fluid_, boundaries_); },
      std::invalid_argument);
}

TEST_F(KEpsilonTest, InitialEddyViscosity) {
  core::flow::KEpsilonOptions options;
  options.initial_k = 1.0;
  options.initial_epsilon = 2.0;
  fluid_.kinematic_viscosity = 1e-3;

  core::flow::KEpsilon model(grid_, fluid_, boundaries_, options);

  // nu_t = C_mu k^2 / epsilon
  EXPECT_NEAR(model.EddyViscosity()(1, 1), 0.09 * 1.0 / 2.0, kTolerance);
}

TEST_F(KEpsilonTest, InletSetsInitialState) {
  boundaries_.west.type = core::flow::BoundaryType::kInlet;
  boundaries_.west.k = 0.5;
  boundaries_.west.epsilon = 0.25;

  core::flow::KEpsilon model(grid_, fluid_, boundaries_);

  EXPECT_NEAR(model.TurbulentKineticEnergy()(2, 2), 0.5, kTolerance);
  EXPECT_NEAR(model.Dissipation()(2, 2), 0.25, kTolerance);
}

TEST_F(KEpsilonTest, WallViscosityInLaminarSublayer) {
  core::flow::KEpsilonOptions options;
  options.initial_k = 1e-8;

  core::flow::KEpsilon model(grid_, fluid_, boundaries_, options);

  EXPECT_NEAR(model.WallViscosity(1, 1, 1e-4), fluid_.kinematic_viscosity,
              kTolerance);
}

TEST_F(KEpsilonTest, WallViscosityInLogLayer) {
  core::flow::KEpsilonOptions options;
  options.initial_k = 0.01;

  core::flow::KEpsilon model(grid_, fluid_, boundaries_, options);

  // y* is about 55, so the wall function raises the effective viscosity
  EXPECT_GT(model.WallViscosity(1, 1, 0.01), fluid_.kinematic_viscosity);
}
