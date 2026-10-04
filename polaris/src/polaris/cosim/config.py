"""Declarative description of a HELICS co-simulation federation.

A :class:`CoSimulation` lists the FMUs to run as federates and the publication/subscription
links between them, independent of how the federation is actually executed. This keeps
scenario definitions reusable (e.g. loaded from a TOML file) and decoupled from
:mod:`polaris.cosim.runner`, which turns a ``CoSimulation`` into running
:class:`~polaris.cosim.federate.FmuFederate` instances.
"""

from __future__ import annotations

import math
import tomllib
from collections.abc import Mapping, Sequence
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, cast


@dataclass(frozen=True)
class FederateSpec:
    """One FMU to run as a HELICS value federate.

    Args:
        name: Unique federate name; also used as the HELICS key prefix for its variables.
        fmu: Path to the FMU (source or binary; source FMUs are compiled when run).
        step_size: HELICS period and FMU communication step.
        substeps: Number of FMU internal steps per HELICS time step.
        start_time: Simulation start time.
        parameters: FMU start-value overrides.
        core_type: HELICS core type for this federate, such as ``zmq`` or ``tcp``.
        core_init: Additional HELICS core initialization arguments.
        compile_sources: Compile a source-code FMU locally before loading.
    """

    name: str
    fmu: str | Path
    step_size: float = 0.01
    substeps: int = 1
    start_time: float = 0.0
    parameters: Mapping[str, float] | None = None
    core_type: str = "zmq"
    core_init: str = ""
    compile_sources: bool = True

    def __post_init__(self) -> None:
        if not self.name:
            raise ValueError("Federate name must not be empty")
        if "." in self.name:
            raise ValueError(f"Federate name '{self.name}' must not contain '.'")
        if not math.isfinite(self.step_size) or self.step_size <= 0:
            raise ValueError(f"Federate '{self.name}': step_size must be a positive finite number")
        if self.substeps < 1:
            raise ValueError(f"Federate '{self.name}': substeps must be at least 1")


@dataclass(frozen=True)
class Connection:
    """A publication-to-subscription link between two federates.

    ``source`` and ``target`` are ``"federate.variable"`` references. The source
    federate's variable is published and the target federate subscribes to it; the
    HELICS key used on the wire is the source reference itself, which is unique as long
    as federate names are unique.
    """

    source: str
    target: str

    def __post_init__(self) -> None:
        for label, reference in (("source", self.source), ("target", self.target)):
            if reference.count(".") != 1 or not all(reference.split(".")):
                raise ValueError(
                    f"Connection {label} '{reference}' must look like 'federate.variable'"
                )

    @property
    def source_federate(self) -> str:
        """Federate name on the publishing side."""
        return self.source.split(".", 1)[0]

    @property
    def source_variable(self) -> str:
        """FMU variable name on the publishing side."""
        return self.source.split(".", 1)[1]

    @property
    def target_federate(self) -> str:
        """Federate name on the subscribing side."""
        return self.target.split(".", 1)[0]

    @property
    def target_variable(self) -> str:
        """FMU variable name on the subscribing side."""
        return self.target.split(".", 1)[1]


@dataclass(frozen=True)
class CoSimulation:
    """A complete, backend-independent description of a HELICS federation.

    Args:
        federates: FMUs to run, each as one federate.
        connections: Publication -> subscription links between federate variables.
        stop_time: Simulation stop time shared by every federate.
        broker_name: HELICS broker name; a unique one is generated when omitted.
        broker_core_type: HELICS core type used to create the broker.
        broker_init: Broker initialization string; defaults to ``--federates=N``.
    """

    federates: Sequence[FederateSpec]
    connections: Sequence[Connection] = field(default_factory=tuple)
    stop_time: float = 1.0
    broker_name: str | None = None
    broker_core_type: str = "zmq"
    broker_init: str | None = None

    def __post_init__(self) -> None:
        # Accept plain mappings (e.g. parsed TOML/JSON) as well as the dataclasses
        # themselves; coerce here so the rest of the class only deals with the latter.
        # The constructor's declared type is the dataclass-only contract; callers that
        # pass mappings (from_dict/from_toml) rely on this runtime coercion.
        object.__setattr__(
            self,
            "federates",
            tuple(
                spec if isinstance(spec, FederateSpec) else FederateSpec(**cast(Any, spec))
                for spec in self.federates
            ),
        )
        object.__setattr__(
            self,
            "connections",
            tuple(
                conn if isinstance(conn, Connection) else Connection(**cast(Any, conn))
                for conn in self.connections
            ),
        )
        if not self.federates:
            raise ValueError("A CoSimulation needs at least one federate")
        names = [spec.name for spec in self.federates]
        if len(set(names)) != len(names):
            raise ValueError(f"Federate names must be unique, got {names}")
        if not math.isfinite(self.stop_time) or self.stop_time <= 0:
            raise ValueError("stop_time must be a positive finite number")
        known = set(names)
        for connection in self.connections:
            for role, reference in (
                ("source", connection.source_federate),
                ("target", connection.target_federate),
            ):
                if reference not in known:
                    raise ValueError(
                        f"Connection {role} federate '{reference}' is not in {sorted(known)}"
                    )

    def outputs_for(self, federate: str) -> dict[str, str]:
        """FMU variable -> HELICS key publications for ``federate``, derived from connections."""
        return {
            c.source_variable: c.source for c in self.connections if c.source_federate == federate
        }

    def inputs_for(self, federate: str) -> dict[str, str]:
        """FMU input -> HELICS key subscriptions for ``federate``, derived from connections."""
        return {
            c.target_variable: c.source for c in self.connections if c.target_federate == federate
        }

    @classmethod
    def from_dict(cls, data: Mapping[str, Any]) -> CoSimulation:
        """Build a :class:`CoSimulation` from a plain mapping, e.g. parsed TOML.

        Expected shape::

            {
                "stop_time": 2.0,
                "broker_name": "optional",
                "federates": [{"name": ..., "fmu": ..., "step_size": ...}, ...],
                "connections": [{"source": "a.y", "target": "b.u"}, ...],
            }
        """
        kwargs: dict[str, Any] = {
            "federates": data.get("federates", []),
            "connections": data.get("connections", []),
        }
        for key in ("stop_time", "broker_name", "broker_core_type", "broker_init"):
            if key in data:
                kwargs[key] = data[key]
        return cls(**kwargs)

    @classmethod
    def from_toml(cls, path: str | Path) -> CoSimulation:
        """Load a :class:`CoSimulation` from a TOML file (see :meth:`from_dict` for the schema)."""
        with Path(path).open("rb") as handle:
            data = tomllib.load(handle)
        return cls.from_dict(data)
