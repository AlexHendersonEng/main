#include "optimisers/solution_bindings.hpp"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "optimisers/solution.hpp"

namespace vanta::bindings::python::optimisers {

void BindSolution(pybind11::module_& m) {
  pybind11::class_<vanta::optimisers::Solution>(m, "Solution")
      .def(pybind11::init<>())
      .def_readwrite("f_val", &vanta::optimisers::Solution::f_val)
      .def_property_readonly(
          "x",
          [](const vanta::optimisers::Solution& self) {
            return pybind11::array(self.x.size(), self.x.data()).attr("copy")();
          })
      .def_readwrite("converged", &vanta::optimisers::Solution::converged)
      .def_readwrite("iters", &vanta::optimisers::Solution::iters)
      .doc() = R"pbdoc(
Optimisation result container.

Represents the outcome of an optimisation run.

Attributes
----------
f_val : float
    Objective function value at the solution.
x : list[float]
    Best solution vector.
converged : bool
    Whether the optimiser met its convergence criterion.
iters : int
    Number of iterations performed.

Convenience
-----------
x_np : numpy.ndarray
    NumPy view of the solution vector.
)pbdoc";
}

}  // namespace vanta::bindings::python::optimisers
