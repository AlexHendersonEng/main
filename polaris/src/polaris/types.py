"""Plain data types shared between the core and backends."""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import StrEnum
from pathlib import Path


class FmuKind(StrEnum):
    """FMI interface type; values match the ``fmuType`` strings compilers expect."""

    CO_SIMULATION = "cs"
    MODEL_EXCHANGE = "me"


@dataclass(frozen=True, slots=True)
class ModelSource:
    """Where a Modelica class lives and what it needs to be loaded."""

    # Fully qualified name, e.g. "Modelica.Blocks.Examples.PID_Controller".
    class_name: str
    # .mo files to load before building.
    files: tuple[Path, ...] = ()
    # Libraries loaded from MODELICAPATH, e.g. "Modelica".
    libraries: tuple[str, ...] = ()
    # Run-time parameter overrides (name -> value).
    parameters: dict[str, float] = field(default_factory=dict)


@dataclass(frozen=True, slots=True)
class SimulationOptions:
    """Solver settings. ``None`` means "let the backend choose"."""

    start_time: float = 0.0
    stop_time: float = 1.0
    step_size: float | None = None
    tolerance: float = 1e-6
    solver: str | None = None
    # Variable names to record; empty means everything.
    outputs: tuple[str, ...] = ()
