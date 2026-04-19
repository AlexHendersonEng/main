#include <pybind11/pybind11.h>

#include "ode/euler_backward_bindings.hpp"
#include "ode/euler_forward.hpp"
#include "ode/euler_forward_bindings.hpp"
#include "ode/runge_kutta_4_bindings.hpp"
#include "ode/solution_bindings.hpp"
#include "optimisers/genetic_algorithm_bindings.hpp"
#include "optimisers/gradient_descent_bindings.hpp"
#include "optimisers/particle_swarm_bindings.hpp"
#include "optimisers/solution_bindings.hpp"

// Python module definition
PYBIND11_MODULE(vanta_core_py, m, pybind11::mod_gil_not_used()) {
  m.doc() = "Vanta Core Python bindings";

  auto m_ode = m.def_submodule("ode", R"pbdoc(
        Ordinary Differential Equation solvers
    )pbdoc");
  vanta::bindings::python::ode::BindSolution(m_ode);
  vanta::bindings::python::ode::BindEulerForward(m_ode);
  vanta::bindings::python::ode::BindRungeKutta4(m_ode);
  vanta::bindings::python::ode::BindEulerBackward(m_ode);

  auto m_optimisers = m.def_submodule("optimisers", R"pbdoc(
        Optimisation algorithms
    )pbdoc");
  vanta::bindings::python::optimisers::BindSolution(m_optimisers);
  vanta::bindings::python::optimisers::BindGDOptions(m_optimisers);
  vanta::bindings::python::optimisers::BindGradientDescent(m_optimisers);
  vanta::bindings::python::optimisers::BindPSOptions(m_optimisers);
  vanta::bindings::python::optimisers::BindParticleSwarm(m_optimisers);
  vanta::bindings::python::optimisers::BindGAOptions(m_optimisers);
  vanta::bindings::python::optimisers::BindGeneticAlgorithm(m_optimisers);
}
