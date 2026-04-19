#ifndef BINDINGS_PYTHON_OPTIMISERS_GENETIC_ALGORITHM_BINDINGS_HPP_
#define BINDINGS_PYTHON_OPTIMISERS_GENETIC_ALGORITHM_BINDINGS_HPP_

#include <pybind11/pybind11.h>

namespace vanta::bindings::python::optimisers {

void BindGAOptions(pybind11::module_& m);

void BindGeneticAlgorithm(pybind11::module_& m);

}  // namespace vanta::bindings::python::optimisers

#endif  // BINDINGS_PYTHON_OPTIMISERS_GENETIC_ALGORITHM_BINDINGS_HPP_
