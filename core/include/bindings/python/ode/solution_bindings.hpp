#pragma once

#include <pybind11/pybind11.h>

#include "ode/solution.hpp"

void BindSolution(pybind11::module_& m);
