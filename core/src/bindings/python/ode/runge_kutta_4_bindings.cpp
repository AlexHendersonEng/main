#include "ode/runge_kutta_4_bindings.hpp"

#include <pybind11/functional.h>
#include <pybind11/numpy.h>

#include "ode/runge_kutta_4.hpp"

void bind_runge_kutta_4(pybind11::module_& m) {
  m.def(
      "runge_kutta_4",
      [](std::function<pybind11::array_t<double>(double,
                                                 pybind11::array_t<double>)>
             f,
         double t0, double t1, pybind11::array_t<double> y0, double h) {
        // Wrap the numpy-compatible callable into the signature runge_kutta_4
        // expects
        auto f_wrapped = [&f](double t, const std::vector<double>& y) {
          pybind11::array_t<double> y_arr(y.size(), y.data());
          pybind11::array_t<double> dy_arr = f(t, y_arr);
          auto buf = dy_arr.request();
          auto* ptr = static_cast<double*>(buf.ptr);
          return std::vector<double>(ptr, ptr + buf.size);
        };

        // Convert y0 from numpy array to std::vector
        auto buf = y0.request();
        auto* ptr = static_cast<double*>(buf.ptr);
        std::vector<double> y0_vec(ptr, ptr + buf.size);

        return runge_kutta_4(f_wrapped, t0, t1, y0_vec, h);
      },
      pybind11::arg("f"), pybind11::arg("t0"), pybind11::arg("t1"),
      pybind11::arg("y0"), pybind11::arg("h"),
      R"pbdoc(
            Solve an ODE using the Runge-Kutta 4th order method.

            Numerically integrates the ODE system

                dy/dt = f(t, y)

            over ``[t0, t1]`` with fixed step size ``h``.

            Parameters
            ----------
            f : Callable[[float, list[float]], list[float]]
                Right-hand side of the ODE. Receives the current time ``t``
                and state vector ``y``, returns the derivative ``dy/dt``.
            t0 : float
                Initial time.
            t1 : float
                Final time.
            y0 : list[float]
                Initial state vector at ``t0``.
            h : float
                Time step size. The last step is shortened automatically to
                land exactly on ``t1``.

            Returns
            -------
            Solution
                Object with attributes ``t`` (time grid) and ``y``
                (corresponding state vectors).

            Notes
            -----
            This method is fourth-order accurate and conditionally stable.
            For stiff problems a smaller ``h`` or an implicit solver should be used.

            Examples
            --------
            Solve the scalar decay equation  dy/dt = -y,  y(0) = 1:

            >>> from simulation_foundry_core_py import runge_kutta_4
            >>> sol = runge_kutta_4(
            ...     f=lambda t, y: [-y[0]],
            ...     t0=0.0, t1=5.0,
            ...     y0=[1.0], h=0.1
            ... )
            >>> sol.t[-1]
            5.0
            >>> sol.y[0]
            [1.0]
        )pbdoc");
}
