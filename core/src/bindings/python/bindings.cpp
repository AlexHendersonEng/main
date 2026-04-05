#include <pybind11/pybind11.h>

#include "ode/euler_backward_bindings.hpp"
#include "ode/euler_forward.hpp"
#include "ode/euler_forward_bindings.hpp"
#include "ode/runge_kutta_4_bindings.hpp"
#include "ode/solution_bindings.hpp"

namespace py = pybind11;

// Python module definition
PYBIND11_MODULE(vanta_core_py, m, pybind11::mod_gil_not_used()) {
  m.doc() = "Vanta Core Python bindings";

  auto m_ode = m.def_submodule("ode", R"pbdoc(
        Ordinary Differential Equation solvers
    )pbdoc");
  BindSolution(m_ode);
  BindEulerForward(m_ode);
  BindRungeKutta4(m_ode);
  BindEulerBackward(m_ode);
}
