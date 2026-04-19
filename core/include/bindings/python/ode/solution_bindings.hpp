#ifndef BINDINGS_PYTHON_ODE_SOLUTION_BINDINGS_HPP_
#define BINDINGS_PYTHON_ODE_SOLUTION_BINDINGS_HPP_

#include <pybind11/pybind11.h>

#include "ode/solution.hpp"

namespace vanta::bindings::python::ode {

void BindSolution(pybind11::module_& m);

}  // namespace vanta::bindings::python::ode

#endif  // BINDINGS_PYTHON_ODE_SOLUTION_BINDINGS_HPP_
