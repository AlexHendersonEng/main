"""Inspecting and validating FMUs (backend independent, built on FMPy)."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path

import fmpy
from fmpy import read_model_description
from fmpy.validation import validate_fmu as _fmpy_validate


class FmuError(Exception):
    """An FMU is unreadable or cannot be run."""


class FmuNotRunnableError(FmuError):
    """The FMU has no binary for this platform (e.g. a source-code FMU)."""


@dataclass(frozen=True)
class FmuVariable:
    name: str
    causality: str
    variability: str
    start: str | None


@dataclass(frozen=True)
class FmuInfo:
    """Summary of an FMU's modelDescription.xml plus where it can run."""

    path: Path
    fmi_version: str
    model_name: str
    co_simulation: bool
    model_exchange: bool
    # FMPy platform names found in the archive, e.g. "win64" or "c-code" (sources).
    platforms: tuple[str, ...]
    variables: tuple[FmuVariable, ...] = field(repr=False, default=())

    @property
    def runnable(self) -> bool:
        """Whether the archive holds a binary for the current machine."""
        return fmpy.platform in self.platforms

    def parameters(self) -> dict[str, str | None]:
        """Tunable parameters and their start values (as strings, per the FMI schema)."""
        return {v.name: v.start for v in self.variables if v.causality == "parameter"}


def inspect_fmu(path: str | Path) -> FmuInfo:
    """Read an FMU's metadata without running it."""
    p = Path(path)
    if not p.is_file():
        raise FmuError(f"FMU not found: {p}")
    try:
        md = read_model_description(str(p), validate=False)
        platforms = tuple(fmpy.supported_platforms(str(p)))
    except Exception as exc:
        raise FmuError(f"Cannot read FMU {p}: {exc}") from exc
    variables = tuple(
        FmuVariable(v.name, str(v.causality), str(v.variability), v.start)
        for v in md.modelVariables
    )
    return FmuInfo(
        path=p,
        fmi_version=str(md.fmiVersion),
        model_name=str(md.modelName),
        co_simulation=md.coSimulation is not None,
        model_exchange=md.modelExchange is not None,
        platforms=platforms,
        variables=variables,
    )


@dataclass(frozen=True)
class ValidationReport:
    """Outcome of :func:`validate_fmu`.

    ``errors`` make the FMU unusable; ``warnings`` are things to be aware of, such as a
    source-only FMU that cannot be run here.
    """

    info: FmuInfo
    errors: tuple[str, ...] = ()
    warnings: tuple[str, ...] = ()

    @property
    def ok(self) -> bool:
        return not self.errors


def validate_fmu(path: str | Path, *, smoke_test: bool = True) -> ValidationReport:
    """Check an FMU's description and, when possible, run a very short simulation.

    Args:
        path: The ``.fmu`` file.
        smoke_test: Also instantiate the FMU and step it briefly. Skipped automatically
            (with a warning) when the archive has no binary for this platform.
    """
    info = inspect_fmu(path)
    errors = [str(problem) for problem in _fmpy_validate(str(info.path))]
    warnings: list[str] = []

    if not (info.co_simulation or info.model_exchange):
        errors.append("FMU supports neither Co-Simulation nor Model Exchange")
    if not info.runnable:
        warnings.append(
            f"No binary for {fmpy.platform}; archive contains "
            f"{', '.join(info.platforms) or 'none'}. "
            "It must be compiled by the importing tool before it can run here."
        )
    elif smoke_test and not errors:
        # Imported here to avoid a circular import (the runtime uses FmuInfo).
        from polaris.fmu_runtime import run_fmu
        from polaris.types import SimulationOptions

        try:
            run_fmu(info.path, SimulationOptions(stop_time=1e-3, step_size=1e-3))
        except Exception as exc:
            errors.append(f"Smoke test simulation failed: {exc}")
    return ValidationReport(info, tuple(errors), tuple(warnings))
