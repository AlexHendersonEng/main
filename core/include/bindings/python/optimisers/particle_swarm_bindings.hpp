#ifndef BINDINGS_PYTHON_OPTIMISERS_PARTICLE_SWARM_BINDINGS_HPP_
#define BINDINGS_PYTHON_OPTIMISERS_PARTICLE_SWARM_BINDINGS_HPP_

#include <pybind11/pybind11.h>

namespace vanta::bindings::python::optimisers {

void BindPSOptions(pybind11::module_& m);

void BindParticleSwarm(pybind11::module_& m);

}  // namespace vanta::bindings::python::optimisers

#endif  // BINDINGS_PYTHON_OPTIMISERS_PARTICLE_SWARM_BINDINGS_HPP_
