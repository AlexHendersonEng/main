#include "optimisers/genetic_algorithm_bindings.hpp"

#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "optimisers/genetic_algorithm.hpp"

namespace vanta::bindings::python::optimisers {

void BindGAOptions(pybind11::module_& m) {
  pybind11::class_<vanta::optimisers::GAOptions>(m, "GAOptions")
      .def(pybind11::init<>())
      .def_readwrite("population_size",
                     &vanta::optimisers::GAOptions::population_size)
      .def_readwrite("max_generations",
                     &vanta::optimisers::GAOptions::max_generations)
      .def_readwrite("crossover_rate",
                     &vanta::optimisers::GAOptions::crossover_rate)
      .def_readwrite("mutation_rate",
                     &vanta::optimisers::GAOptions::mutation_rate)
      .def_readwrite("mutation_strength",
                     &vanta::optimisers::GAOptions::mutation_strength)
      .def_readwrite("tournament_size",
                     &vanta::optimisers::GAOptions::tournament_size)
      .def_readwrite("tolerance", &vanta::optimisers::GAOptions::tolerance)
      .doc() = R"pbdoc(
Genetic Algorithm configuration options.

Controls population evolution, mutation, crossover, and stopping criteria.

Attributes
----------
population_size : int
    Number of individuals per generation.
max_generations : int
    Maximum number of generations.
crossover_rate : float
    Probability of crossover between parents.
mutation_rate : float
    Probability of mutating each gene.
mutation_strength : float
    Relative magnitude of mutation.
tournament_size : int
    Number of candidates in tournament selection.
tolerance : float
    Convergence threshold on objective value.
)pbdoc";
}

void BindGeneticAlgorithm(pybind11::module_& m) {
  m.def(
      "genetic_algorithm",
      [](std::function<double(pybind11::array_t<double>)> f,
         pybind11::array_t<double> lower_bounds,
         pybind11::array_t<double> upper_bounds,
         vanta::optimisers::GAOptions opts) {
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

        return vanta::optimisers::GeneticAlgorithm(f_wrapped, lb, ub, opts);
      },
      pybind11::arg("f"), pybind11::arg("lower_bounds"),
      pybind11::arg("upper_bounds"),
      pybind11::arg("opts") = vanta::optimisers::GAOptions{},
      R"pbdoc(
Minimise a function using a genetic algorithm.

This optimiser evolves a population of candidate solutions using
selection, crossover, mutation, and elitism.

Parameters
----------
f : Callable[[array_like], float]
    Objective function to minimise.
lower_bounds : array_like
    Lower bound for each parameter.
upper_bounds : array_like
    Upper bound for each parameter.
opts : GAOptions
    Genetic algorithm configuration parameters.

Returns
-------
Solution
    Best solution found, including:
    - x (best parameters)
    - f_val (objective value)
    - converged (bool)
    - iters (generations run)

Notes
-----
- Assumes a bounded continuous search space.
- Uses tournament selection and BLX-α crossover.
- Randomness is internal (not externally seeded).
)pbdoc");
}

}  // namespace vanta::bindings::python::optimisers
