#include "ode/euler_backward_bindings.hpp"

#include <pybind11/functional.h>
#include <pybind11/numpy.h>

#include "ode/euler_backward.hpp"

namespace vanta::bindings::python::ode {

void BindEulerBackward(pybind11::module_& m) {
  m.def(
      "euler_backward",
      [](std::function<pybind11::array_t<double>(double,
                                                 pybind11::array_t<double>)>
             f,
         double t0, double t1, pybind11::array_t<double> y0, double h) {
        // Wrap the numpy-compatible callable into the signature EulerBackward
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

        return vanta::ode::EulerBackward(f_wrapped, t0, t1, y0_vec, h);
      },
      pybind11::arg("f"), pybind11::arg("t0"), pybind11::arg("t1"),
      pybind11::arg("y0"), pybind11::arg("h"),
      R"pbdoc(
            Solve an ODE using the backward (implicit) Euler method.

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
            This method is first-order accurate but unconditionally stable,
            making it suitable for stiff ODEs.

            Examples
            --------
            Solve the scalar decay equation  dy/dt = -y,  y(0) = 1:

            >>> from vanta_core_py import euler_backward
            >>> sol = euler_backward(
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

}  // namespace vanta::bindings::python::ode
