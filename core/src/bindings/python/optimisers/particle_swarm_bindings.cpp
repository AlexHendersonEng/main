#include "optimisers/particle_swarm_bindings.hpp"

#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "optimisers/particle_swarm.hpp"

namespace vanta::bindings::python::optimisers {

void BindPSOptions(pybind11::module_& m) {
  pybind11::class_<vanta::optimisers::PSOptions>(m, "PSOptions")
      .def(pybind11::init<>())
      .def_readwrite("n_particles", &vanta::optimisers::PSOptions::n_particles)
      .def_readwrite("max_iters", &vanta::optimisers::PSOptions::max_iters)
      .def_readwrite("w", &vanta::optimisers::PSOptions::w)
      .def_readwrite("c1", &vanta::optimisers::PSOptions::c1)
      .def_readwrite("c2", &vanta::optimisers::PSOptions::c2)
      .def_readwrite("tolerance", &vanta::optimisers::PSOptions::tolerance)
      .doc() = R"pbdoc(
Particle Swarm Optimisation configuration options.

Controls swarm size, inertia, learning coefficients, and stopping criteria.

Attributes
----------
n_particles : int
    Number of particles in the swarm.
max_iters : int
    Maximum number of iterations.
w : float
    Inertia weight (momentum).
c1 : float
    Cognitive coefficient (personal best attraction).
c2 : float
    Social coefficient (global best attraction).
tolerance : float
    Convergence threshold on objective value.
)pbdoc";
}

void BindParticleSwarm(pybind11::module_& m) {
  m.def(
      "particle_swarm",
      [](std::function<double(pybind11::array_t<double>)> f,
         pybind11::array_t<double> lower_bounds,
         pybind11::array_t<double> upper_bounds,
         vanta::optimisers::PSOptions opts) {
        // Wrap objective: numpy -> std::vector
        auto f_wrapped = [&f](const std::vector<double>& x_vec) {
          pybind11::array_t<double> x_arr(x_vec.size(), x_vec.data());
          return f(x_arr);
        };

        // Convert bounds
        auto lb_buf = lower_bounds.request();
        auto ub_buf = upper_bounds.request();

        auto* lb_ptr = static_cast<double*>(lb_buf.ptr);
        auto* ub_ptr = static_cast<double*>(ub_buf.ptr);

        std::vector<double> lb(lb_ptr, lb_ptr + lb_buf.size);
        std::vector<double> ub(ub_ptr, ub_ptr + ub_buf.size);

        return vanta::optimisers::ParticleSwarm(f_wrapped, lb, ub, opts);
      },
      pybind11::arg("f"), pybind11::arg("lower_bounds"),
      pybind11::arg("upper_bounds"),
      pybind11::arg("opts") = vanta::optimisers::PSOptions{},
      R"pbdoc(
Minimise a function using Particle Swarm Optimisation (PSO).

PSO is a population-based optimisation algorithm where particles
move through the search space guided by personal and global bests.

Parameters
----------
f : Callable[[array_like], float]
    Objective function to minimise.
lower_bounds : array_like
    Lower bounds for each dimension.
upper_bounds : array_like
    Upper bounds for each dimension.
opts : PSOptions
    PSO configuration parameters.

Returns
-------
Solution
    Best solution found:
    - x (best parameters)
    - f_val (objective value)
    - converged (bool)
    - iters (iterations run)

Notes
-----
- Uses inertia + cognitive + social velocity updates.
- Positions are clamped to bounds each iteration.
- Randomness is internal (not externally seeded).
)pbdoc");
}

}  // namespace vanta::bindings::python::optimisers
