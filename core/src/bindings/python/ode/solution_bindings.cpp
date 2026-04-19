#include "ode/solution_bindings.hpp"

#include <pybind11/numpy.h>

namespace vanta::bindings::python::ode {

void BindSolution(pybind11::module_& m) {
  pybind11::class_<vanta::ode::Solution>(m, "Solution", R"pbdoc(
        Container for a numerical ODE solution.
 
        Attributes
        ----------
        t : list[float]
            Time points at which the solution was evaluated.
        y : list[list[float]]
            State vectors corresponding to each time point.
            ``y[i]`` is the state vector at time ``t[i]``.
    )pbdoc")
      .def(pybind11::init<>())
      .def(pybind11::init<>())
      .def_property(
          "t",
          [](const vanta::ode::Solution& s) {
            return pybind11::array_t<double>(s.t.size(), s.t.data());
          },
          [](vanta::ode::Solution& s, pybind11::array_t<double> arr) {
            auto buf = arr.request();
            auto* ptr = static_cast<double*>(buf.ptr);
            s.t.assign(ptr, ptr + buf.size);
          },
          "Time points at which the solution is evaluated.")
      .def_property(
          "y",
          [](const vanta::ode::Solution& s) {
            if (s.y.empty()) return pybind11::array_t<double>({0, 0});
            size_t rows = s.y.size();
            size_t cols = s.y[0].size();
            pybind11::array_t<double> arr({rows, cols});
            auto buf = arr.mutable_unchecked<2>();
            for (size_t i = 0; i < rows; i++)
              for (size_t j = 0; j < cols; j++) buf(i, j) = s.y[i][j];
            return arr;
          },
          [](vanta::ode::Solution& s, pybind11::array_t<double> arr) {
            auto buf = arr.request();
            if (buf.ndim != 2) throw std::runtime_error("y must be a 2D array");
            auto* ptr = static_cast<double*>(buf.ptr);
            size_t rows = buf.shape[0];
            size_t cols = buf.shape[1];
            s.y.assign(rows, std::vector<double>(cols));
            for (size_t i = 0; i < rows; i++)
              for (size_t j = 0; j < cols; j++) s.y[i][j] = ptr[i * cols + j];
          },
          "Solution vectors corresponding to each time point.")
      .def("__repr__",
           [](const vanta::ode::Solution& s) {
             return "<Solution: " + std::to_string(s.t.size()) + " time steps>";
           })
      .def("__len__", [](const vanta::ode::Solution& s) { return s.t.size(); });
}

}  // namespace vanta::bindings::python::ode
