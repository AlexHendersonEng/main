"""Run FMUs with FMPy, independent of which compiler produced them."""

from __future__ import annotations

from collections.abc import Mapping
from pathlib import Path

import numpy as np
from fmpy import simulate_fmu

from polaris.fmu import FmuError, FmuNotRunnableError, compile_source_fmu, inspect_fmu
from polaris.result import Result
from polaris.types import SimulationOptions


def run_fmu(
    path: str | Path,
    options: SimulationOptions | None = None,
    parameters: Mapping[str, float] | None = None,
    *,
    substeps: int = 1,
    compile_sources: bool = True,
) -> Result:
    """Simulate an FMU and return a :class:`~polaris.result.Result`.

    Args:
        path: The ``.fmu`` file. Must contain a binary for this platform.
        options: Start/stop time, output step and tolerance. Without ``step_size`` the
            run is divided into 500 steps.
        parameters: Start-value overrides applied before initialisation.
        substeps: Co-Simulation FMUs advance with a fixed communication step, and
            OpenModelica's use forward Euler, so accuracy depends on that step. With
            ``substeps=n`` the FMU is stepped ``n`` times per output point and only the
            output points are kept.
        compile_sources: Source-code FMUs (e.g. from rumoca) are compiled with the local C
            compiler into a temporary copy first. Set to ``False`` to raise instead.
    """
    opts = options or SimulationOptions()
    info = inspect_fmu(path)
    if not info.runnable and compile_sources and "c-code" in info.platforms:
        try:
            info = inspect_fmu(compile_source_fmu(info.path))
        except FmuError as exc:
            raise FmuNotRunnableError(str(exc)) from exc
    if not info.runnable:
        raise FmuNotRunnableError(
            f"{info.path.name} has no binary for this platform (contains: "
            f"{', '.join(info.platforms) or 'nothing'}). Source-code FMUs must be compiled first."
        )
    if substeps < 1:
        raise ValueError("substeps must be at least 1")

    span = opts.stop_time - opts.start_time
    step = opts.step_size or span / 500
    names = {v.name for v in info.variables}
    unknown = [k for k in (parameters or {}) if k not in names]
    if unknown:
        raise KeyError(f"Unknown FMU variable(s): {', '.join(unknown)}")

    # FMPy records only causality=output variables by default; record every time-varying
    # variable so results match what the compiler backends return.
    outputs = list(opts.outputs) or [
        v.name for v in info.variables if v.causality != "parameter" and v.variability != "fixed"
    ]
    raw = simulate_fmu(
        str(info.path),
        start_time=opts.start_time,
        stop_time=opts.stop_time,
        output_interval=step / substeps,
        relative_tolerance=opts.tolerance,
        start_values=dict(parameters or {}),
        output=outputs,
        fmi_type="CoSimulation" if info.co_simulation else "ModelExchange",
    )
    # With substeps, keep every n-th row so output points stay on the requested grid.
    rows = raw[::substeps]
    variables = {n: np.asarray(rows[n], dtype=float) for n in rows.dtype.names or () if n != "time"}
    meta = {"backend": "fmu", "model": info.model_name, "fmi": info.fmi_version}
    return Result(np.asarray(rows["time"], dtype=float), variables, meta)
