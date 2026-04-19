#include "optimisers/gradient_descent_bindings.hpp"

#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "optimisers/gradient_descent.hpp"

namespace vanta::bindings::python::optimisers {

void BindGDOptions(pybind11::module_& m) {
  pybind11::class_<vanta::optimisers::GDOptions>(m, "GDOptions")
      .def(pybind11::init<>())
      .def_readwrite("learning_rate",
                     &vanta::optimisers::GDOptions::learning_rate)
      .def_readwrite("max_iters", &vanta::optimisers::GDOptions::max_iters)
      .def_readwrite("tolerance", &vanta::optimisers::GDOptions::tolerance)
      .def_readwrite("finite_difference_step",
                     &vanta::optimisers::GDOptions::finite_difference_step)
      .def_readwrite("lower_bounds",
                     &vanta::optimisers::GDOptions::lower_bounds)
      .def_readwrite("upper_bounds",
                     &vanta::optimisers::GDOptions::upper_bounds)
      .doc() = R"pbdoc(
Gradient Descent configuration options.

Controls step size, stopping criteria, and optional bound constraints.

Attributes
----------
learning_rate : float
    Step size used in parameter updates.
max_iters : int
    Maximum number of iterations.
tolerance : float
    Convergence threshold on gradient norm.
finite_difference_step : float
    Step size used for numerical gradient approximation.
lower_bounds : list[float]
    Optional per-variable lower bounds (empty = none).
upper_bounds : list[float]
    Optional per-variable upper bounds (empty = none).
)pbdoc";
}

void BindGradientDescent(pybind11::module_& m) {
  m.def(
      "gradient_descent",
      [](std::function<double(pybind11::array_t<double>)> f,
         pybind11::array_t<double> x0, pybind11::object grad_f_obj,
         vanta::optimisers::GDOptions opts) {
        // Wrap f: numpy -> std::vector
        auto f_wrapped = [&f](const std::vector<double>& x_vec) {
          pybind11::array_t<double> x_arr(x_vec.size(), x_vec.data());
          return f(x_arr);
        };

        // Convert x0 to std::vector
        auto buf = x0.request();
        auto* ptr = static_cast<double*>(buf.ptr);
        std::vector<double> x_vec(ptr, ptr + buf.size);

        // Handle optional gradient
        std::function<std::vector<double>(const std::vector<double>&)>
            grad_f_wrapped;

        if (!grad_f_obj.is_none()) {
          auto grad_f_py =
              grad_f_obj.cast<std::function<pybind11::array_t<double>(
                  pybind11::array_t<double>)>>();

          grad_f_wrapped = [grad_f_py](const std::vector<double>& x_vec_inner) {
            pybind11::array_t<double> x_arr(x_vec_inner.size(),
                                            x_vec_inner.data());
            pybind11::array_t<double> grad_arr = grad_f_py(x_arr);

            auto gbuf = grad_arr.request();
            auto* gptr = static_cast<double*>(gbuf.ptr);
            return std::vector<double>(gptr, gptr + gbuf.size);
          };
        }

        // Call core function
        return vanta::optimisers::GradientDescent(f_wrapped, x_vec,
                                                  grad_f_wrapped, opts);
      },
      pybind11::arg("f"), pybind11::arg("x0"),
      pybind11::arg("grad_f") = pybind11::none(), pybind11::arg("opts"),
      R"pbdoc(
Minimise a scalar function using gradient descent.

This function attempts to find a local minimum of a scalar-valued
function ``f(x)`` using iterative gradient-based updates.

Parameters
----------
f : Callable[[array_like], float]
    Objective function to minimise.
x0 : array_like
    Initial guess for the parameters.
grad_f : Optional[Callable[[array_like], array_like]]
    Gradient of the objective function. If not provided,
    a forward finite difference approximation is used.
opts : GDOptions
    Gradient descent configuration options (learning rate,
    tolerance, bounds, etc.).

Returns
-------
Solution
    Object containing:
    - ``x`` : final parameter values
    - ``f_val`` : function value at ``x``
    - ``converged`` : whether convergence was reached
    - ``iters`` : number of iterations performed

Notes
-----
- If ``grad_f`` is not supplied, numerical differentiation is used,
    which may be slower and less accurate.
- Supports optional box constraints via ``opts.lower_bounds`` and
    ``opts.upper_bounds``.

Examples
--------
>>> from vanta_core_py import gradient_descent, GDOptions
>>> import numpy as np
>>>
>>> f = lambda x: (x[0] - 3.0)**2
>>> grad = lambda x: np.array([2*(x[0] - 3.0)])
>>>
>>> opts = GDOptions()
>>> opts.learning_rate = 0.1
>>> opts.max_iters = 100
>>>
>>> sol = gradient_descent(f, np.array([0.0]), grad, opts)
>>> sol.x
[3.0]
)pbdoc");
}

}  // namespace vanta::bindings::python::optimisers
