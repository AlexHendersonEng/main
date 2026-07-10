#ifndef BINDINGS_PYTHON_OPTIMISERS_SOLUTION_BINDINGS_HPP_
#define BINDINGS_PYTHON_OPTIMISERS_SOLUTION_BINDINGS_HPP_

#include <pybind11/pybind11.h>

namespace core::bindings::python::optimisers {

void BindSolution(pybind11::module_& m);

}  // namespace core::bindings::python::optimisers

#endif  // BINDINGS_PYTHON_OPTIMISERS_SOLUTION_BINDINGS_HPP_
