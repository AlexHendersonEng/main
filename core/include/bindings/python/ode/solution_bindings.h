#pragma once

#include <pybind11/pybind11.h>

#include "ode/solution.h"

void bind_solution(pybind11::module_& m);
