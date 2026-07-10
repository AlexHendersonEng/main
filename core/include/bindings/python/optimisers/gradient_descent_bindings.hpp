#ifndef BINDINGS_PYTHON_OPTIMISERS_GRADIENT_DESCENT_BINDINGS_HPP_
#define BINDINGS_PYTHON_OPTIMISERS_GRADIENT_DESCENT_BINDINGS_HPP_

#include <pybind11/pybind11.h>

namespace core::bindings::python::optimisers {

void BindGDOptions(pybind11::module_& m);

void BindGradientDescent(pybind11::module_& m);

}  // namespace core::bindings::python::optimisers

#endif  // BINDINGS_PYTHON_OPTIMISERS_GRADIENT_DESCENT_BINDINGS_HPP_
