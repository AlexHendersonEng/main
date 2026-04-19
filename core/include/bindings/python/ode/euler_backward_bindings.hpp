#ifndef BINDINGS_PYTHON_ODE_EULER_BACKWARD_BINDINGS_HPP_
#define BINDINGS_PYTHON_ODE_EULER_BACKWARD_BINDINGS_HPP_

#include <pybind11/pybind11.h>

namespace vanta::bindings::python::ode {

void BindEulerBackward(pybind11::module_& m);

}  // namespace vanta::bindings::python::ode

#endif  // BINDINGS_PYTHON_ODE_EULER_BACKWARD_BINDINGS_HPP_
