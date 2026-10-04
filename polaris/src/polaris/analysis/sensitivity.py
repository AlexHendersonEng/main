"""Parameter sensitivity analysis.

Both analyses only need ``Model.simulate``, so they work with every backend. Runs are
independent, so they are executed in a thread pool (the work happens in compiler
subprocesses, which release the GIL).
"""

from __future__ import annotations

from collections.abc import Callable, Mapping, Sequence
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, replace
from typing import Any

import numpy as np
import pandas as pd

from polaris.backends import Backend
from polaris.model import Model, resolve_backend
from polaris.result import Result
from polaris.types import SimulationOptions

# Used when no step_size is given, so every run shares one output grid.
_DEFAULT_POINTS = 500


@dataclass(frozen=True)
class Sensitivity:
    """Local sensitivities d(output)/d(parameter) around a nominal point.

    ``gradients[parameter][output]`` is the derivative trajectory on ``time``.
    """

    parameters: tuple[str, ...]
    outputs: tuple[str, ...]
    time: np.ndarray
    nominal: Result
    gradients: dict[str, dict[str, np.ndarray]]
    nominal_values: dict[str, float]

    def final(self) -> pd.DataFrame:
        """Derivatives at the last time point; rows are outputs, columns parameters."""
        return pd.DataFrame(
            {p: {o: self.gradients[p][o][-1] for o in self.outputs} for p in self.parameters}
        )

    def normalized_final(self) -> pd.DataFrame:
        """Relative sensitivities (p / y) * dy/dp at the end time, comparable across units.

        Entries are NaN where the output is zero, since the ratio is undefined there.
        """

        def ratio(p: str, o: str) -> float:
            y = float(self.nominal[o][-1])
            return float(self.gradients[p][o][-1]) * self.nominal_values[p] / y if y else np.nan

        return pd.DataFrame({p: {o: ratio(p, o) for o in self.outputs} for p in self.parameters})


@dataclass(frozen=True)
class GlobalSensitivity:
    """Variance- or screening-based global sensitivity indices.

    ``indices`` has one row per parameter: Sobol gives ``S1``/``ST`` (first-order and
    total) with confidence intervals; Morris gives ``mu``, ``mu_star`` and ``sigma``.
    """

    method: str
    parameters: tuple[str, ...]
    indices: pd.DataFrame
    samples: np.ndarray
    responses: np.ndarray


def _run_all(
    model: Model,
    parameter_sets: Sequence[Mapping[str, float]],
    options: SimulationOptions,
    backend: Backend,
    workers: int,
) -> list[Result]:
    """Simulate each parameter set, preserving order."""
    if workers < 1:
        raise ValueError("workers must be at least 1")

    def run(values: Mapping[str, float]) -> Result:
        return model.simulate(options, parameters=dict(values), backend=backend)

    if workers == 1:
        return [run(v) for v in parameter_sets]
    with ThreadPoolExecutor(max_workers=workers) as pool:
        return list(pool.map(run, parameter_sets))


def _with_grid(options: SimulationOptions) -> SimulationOptions:
    """Ensure an explicit step size so perturbed runs can be compared point by point."""
    if options.step_size:
        return options
    span = options.stop_time - options.start_time
    return replace(options, step_size=span / _DEFAULT_POINTS)


def local_sensitivity(
    model: Model,
    parameters: Mapping[str, float],
    outputs: Sequence[str],
    options: SimulationOptions | None = None,
    *,
    rel_step: float = 1e-3,
    central: bool = True,
    backend: str | Backend | None = None,
    workers: int = 1,
) -> Sensitivity:
    """Finite-difference sensitivities of ``outputs`` to ``parameters``.

    Args:
        model: Model to analyse.
        parameters: Parameter name -> nominal value to linearise around. The values are
            given explicitly because backends do not all expose parameter defaults.
        outputs: Variables whose trajectories are differentiated.
        options: Simulation settings; a fixed output grid is enforced.
        rel_step: Perturbation relative to the nominal value (absolute when it is zero).
        central: Central differences (2 runs/parameter, second order) instead of forward
            differences (1 run/parameter, first order).
        workers: Concurrent simulations.
    """
    if not parameters or not outputs:
        raise ValueError("parameters and outputs must not be empty")
    opts = replace(_with_grid(options or SimulationOptions()), outputs=tuple(outputs))
    be = resolve_backend(backend or model.backend)

    steps = {p: rel_step * abs(v) if v != 0 else rel_step for p, v in parameters.items()}
    runs: list[dict[str, float]] = [dict(parameters)]
    for p, v in parameters.items():
        runs.append({**parameters, p: v + steps[p]})
        if central:
            runs.append({**parameters, p: v - steps[p]})
    results = _run_all(model, runs, opts, be, workers)

    nominal = results[0]
    gradients: dict[str, dict[str, np.ndarray]] = {}
    cursor = 1
    for p in parameters:
        up = results[cursor]
        down = results[cursor + 1] if central else None
        cursor += 2 if central else 1
        gradients[p] = {}
        for o in outputs:
            # Interpolate onto the nominal grid in case a backend returned a different one.
            y_up = np.interp(nominal.time, up.time, up[o])
            if down is not None:
                y_down = np.interp(nominal.time, down.time, down[o])
                gradients[p][o] = (y_up - y_down) / (2 * steps[p])
            else:
                gradients[p][o] = (y_up - nominal[o]) / steps[p]
    return Sensitivity(
        tuple(parameters), tuple(outputs), nominal.time, nominal, gradients, dict(parameters)
    )


def global_sensitivity(
    model: Model,
    bounds: Mapping[str, tuple[float, float]],
    output: str,
    options: SimulationOptions | None = None,
    *,
    method: str = "sobol",
    n: int = 64,
    reducer: Callable[[Result], float] | None = None,
    backend: str | Backend | None = None,
    workers: int = 1,
    seed: int | None = None,
) -> GlobalSensitivity:
    """Global sensitivity of a scalar response to parameters varied over ``bounds``.

    Args:
        bounds: Parameter name -> (lower, upper).
        output: Variable the response is computed from.
        method: ``"sobol"`` (variance decomposition) or ``"morris"`` (screening).
        n: Base sample size. Sobol needs ``n * (D + 2)`` runs and Morris ``n * (D + 1)``,
            for ``D`` parameters, so keep it modest for slow backends. Powers of two
            give the best Sobol convergence.
        reducer: Maps a Result to the scalar response; defaults to the final value of
            ``output``.
        seed: Seed for reproducible sampling.
    """
    # Imported lazily: SALib pulls in scipy, which is slow to import.
    from SALib.analyze import morris as morris_analyze
    from SALib.analyze import sobol as sobol_analyze
    from SALib.sample import morris as morris_sample
    from SALib.sample import sobol as sobol_sample

    if method not in ("sobol", "morris"):
        raise ValueError(f"Unknown method {method!r}; use 'sobol' or 'morris'")
    names = list(bounds)
    if not names:
        raise ValueError("bounds must not be empty")
    problem: dict[str, Any] = {
        "num_vars": len(names),
        "names": names,
        "bounds": [list(bounds[k]) for k in names],
    }
    opts = replace(options or SimulationOptions(), outputs=(output,))
    be = resolve_backend(backend or model.backend)
    reduce = reducer or (lambda r: float(r[output][-1]))

    if method == "sobol":
        samples = sobol_sample.sample(problem, n, calc_second_order=False, seed=seed)
    else:
        samples = morris_sample.sample(problem, n, seed=seed)
    sets = [dict(zip(names, row, strict=True)) for row in samples]
    responses = np.array([reduce(r) for r in _run_all(model, sets, opts, be, workers)])

    if method == "sobol":
        out = sobol_analyze.analyze(problem, responses, calc_second_order=False)
        table = pd.DataFrame(
            {k: out[k] for k in ("S1", "S1_conf", "ST", "ST_conf")}, index=pd.Index(names)
        )
    else:
        out = morris_analyze.analyze(problem, samples, responses)
        table = pd.DataFrame(
            {k: out[k] for k in ("mu", "mu_star", "sigma", "mu_star_conf")}, index=pd.Index(names)
        )
    return GlobalSensitivity(method, tuple(names), table, samples, responses)
