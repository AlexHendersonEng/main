"""Polaris: build, simulate and analyse Modelica models across compiler backends."""

from polaris.model import Model
from polaris.result import Result
from polaris.types import FmuKind, SimulationOptions

__version__ = "0.1.0"

__all__ = ["FmuKind", "Model", "Result", "SimulationOptions", "__version__"]
