#include <iostream>
#include <string>

#include "flow/rans_solver.hpp"
#include "flow/vtk_writer.hpp"

namespace {

// Turbulent flow over a backward-facing step using the k-epsilon model
void SolveBackwardFacingStep(const std::string& path) {
  // Domain with a solid block forming the step
  core::flow::Grid grid(120, 40, 6.0, 1.0);
  grid.SetSolidBlock(0, 20, 0, 20);

  // Solver configuration
  core::flow::RansSolverConfig config;
  config.fluid.kinematic_viscosity = 1e-5;
  config.turbulence_model = core::flow::TurbulenceModelType::kKEpsilon;
  config.boundaries.west.type = core::flow::BoundaryType::kInlet;
  config.boundaries.west.u = 1.0;
  config.boundaries.west.k = 3.75e-3;
  config.boundaries.west.epsilon = 1e-3;
  config.boundaries.east.type = core::flow::BoundaryType::kOutlet;
  config.max_iterations = 5000;

  // Solve
  core::flow::RansSolver solver(grid, config);
  const core::flow::SolveResult result = solver.Solve();
  std::cout << "Converged: " << (result.converged ? "yes" : "no") << " after "
            << result.iterations << " iterations\n";

  // Write results
  core::flow::VtkWriter writer(grid);
  writer.AddVector("velocity", solver.CellU(), solver.CellV());
  writer.AddScalar("pressure", solver.Pressure());
  writer.AddScalar("k", solver.TurbulentKineticEnergy());
  writer.AddScalar("epsilon", solver.Dissipation());
  writer.AddScalar("nut", solver.EddyViscosity());
  writer.Write(path);
  std::cout << "Results written to " << path << std::endl;
}

// Laminar lid-driven cavity at Re = 100
void SolveLidDrivenCavity(const std::string& path) {
  // Domain
  core::flow::Grid grid(64, 64, 1.0, 1.0);

  // Solver configuration
  core::flow::RansSolverConfig config;
  config.fluid.kinematic_viscosity = 0.01;
  config.boundaries.north.u = 1.0;
  config.max_iterations = 4000;

  // Solve
  core::flow::RansSolver solver(grid, config);
  const core::flow::SolveResult result = solver.Solve();
  std::cout << "Converged: " << (result.converged ? "yes" : "no") << " after "
            << result.iterations << " iterations\n";

  // Write results
  core::flow::VtkWriter writer(grid);
  writer.AddVector("velocity", solver.CellU(), solver.CellV());
  writer.AddScalar("pressure", solver.Pressure());
  writer.Write(path);
  std::cout << "Results written to " << path << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
  // Usage: flow [step|cavity] [output.vtp]
  const std::string case_name = argc > 1 ? argv[1] : "step";
  const std::string path = argc > 2 ? argv[2] : case_name + ".vtp";

  if (case_name == "step") {
    SolveBackwardFacingStep(path);
  } else if (case_name == "cavity") {
    SolveLidDrivenCavity(path);
  } else {
    std::cerr << "Unknown case '" << case_name << "'. Use 'step' or 'cavity'."
              << std::endl;
    return 1;
  }

  return 0;
}
