"""Plain data types shared between the core and backends."""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import StrEnum
from pathlib import Path


class FmuKind(StrEnum):
    CO_SIMULATION = "cs"
    MODEL_EXCHANGE = "me"


@dataclass(frozen=True, slots=True)
class ModelSource:
    """Where a Modelica class lives and what it needs to be loaded."""

    class_name: str
    files: tuple[Path, ...] = ()
    libraries: tuple[str, ...] = ()
    parameters: dict[str, float] = field(default_factory=dict)


@dataclass(frozen=True, slots=True)
class SimulationOptions:
    start_time: float = 0.0
    stop_time: float = 1.0
    step_size: float | None = None
    tolerance: float = 1e-6
    solver: str | None = None
    outputs: tuple[str, ...] = ()
