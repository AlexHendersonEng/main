#include <pybind11/pybind11.h>

#include "ode/euler_backward_bindings.hpp"
#include "ode/euler_forward.hpp"
#include "ode/euler_forward_bindings.hpp"
#include "ode/runge_kutta_4_bindings.hpp"
#include "ode/solution_bindings.hpp"

namespace py = pybind11;

// Python module definition
PYBIND11_MODULE(simulation_foundry_core_py, m, pybind11::mod_gil_not_used()) {
  m.doc() = "Simulation Foundry Core Python bindings";

  auto m_ode = m.def_submodule("ode", R"pbdoc(
        Ordinary Differential Equation solvers
    )pbdoc");
  bind_solution(m_ode);
  bind_euler_forward(m_ode);
  bind_runge_kutta_4(m_ode);
  bind_euler_backward(m_ode);
}
