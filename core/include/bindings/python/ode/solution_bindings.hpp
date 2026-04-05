#ifndef BINDINGS_PYTHON_ODE_SOLUTION_BINDINGS_HPP_
#define BINDINGS_PYTHON_ODE_SOLUTION_BINDINGS_HPP_

#include <pybind11/pybind11.h>

#include "ode/solution.hpp"

void BindSolution(pybind11::module_& m);

#endif  // BINDINGS_PYTHON_ODE_SOLUTION_BINDINGS_HPP_
