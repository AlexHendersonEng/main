"""HELICS-backed FMU co-simulation primitives."""

from polaris.cosim.config import Connection, CoSimulation, FederateSpec
from polaris.cosim.federate import FmuFederate
from polaris.cosim.runner import run_cosimulation
from polaris.cosim.scale import ScalingMeasurement, benchmark_cosimulation, replicate_federate

__all__ = [
    "CoSimulation",
    "Connection",
    "FederateSpec",
    "FmuFederate",
    "ScalingMeasurement",
    "benchmark_cosimulation",
    "replicate_federate",
    "run_cosimulation",
]
