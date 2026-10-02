"""Backend contract: capability flags, the abstract base class and shared errors."""

from __future__ import annotations

from abc import ABC, abstractmethod
from enum import Flag, auto
from pathlib import Path
from typing import ClassVar

from polaris.backends.versioning import Version
from polaris.types import FmuKind, ModelSource, SimulationOptions


class Capability(Flag):
    NONE = 0
    SIMULATE = auto()
    EXPORT_FMU = auto()
    SYMBOLIC_JACOBIAN = auto()


class BackendError(Exception):
    """Base class for backend failures."""


class BackendUnavailableError(BackendError):
    """The backend's compiler or bindings could not be found."""


class UnsupportedCapabilityError(BackendError):
    """The backend does not implement the requested operation."""


class Backend(ABC):
    """A Modelica compiler/simulator.

    Subclasses declare ``name`` and ``capabilities`` and override only the
    operations they support. Unsupported operations raise
    ``UnsupportedCapabilityError`` so callers can fall back (e.g. to the FMU path).
    """

    name: ClassVar[str]
    capabilities: ClassVar[Capability] = Capability.NONE

    @abstractmethod
    def is_available(self) -> bool:
        """Whether the compiler can be used on this machine."""

    @abstractmethod
    def version(self) -> Version:
        """Detected compiler version; raises BackendUnavailableError if unavailable."""

    def supports(self, capability: Capability) -> bool:
        return capability in self.capabilities

    def simulate(self, source: ModelSource, options: SimulationOptions, work_dir: Path) -> Path:
        """Build and run the model, returning the path of a result file (CSV)."""
        raise self._unsupported(Capability.SIMULATE)

    def export_fmu(
        self,
        source: ModelSource,
        kind: FmuKind,
        fmi_version: str,
        work_dir: Path,
    ) -> Path:
        """Export an FMU, returning its path."""
        raise self._unsupported(Capability.EXPORT_FMU)

    def symbolic_jacobian(self, source: ModelSource, work_dir: Path) -> dict[str, dict[str, str]]:
        """Return the symbolic state-space Jacobian blocks (A, B, C, D) as expressions."""
        raise self._unsupported(Capability.SYMBOLIC_JACOBIAN)

    def _unsupported(self, capability: Capability) -> UnsupportedCapabilityError:
        return UnsupportedCapabilityError(
            f"Backend '{self.name}' does not support {capability.name}"
        )
