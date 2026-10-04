"""Step-by-step simulation of a Co-Simulation FMU that can be steered while it runs."""

from __future__ import annotations

import shutil
import tempfile
from collections.abc import Callable, Mapping, Sequence
from pathlib import Path
from types import TracebackType
from typing import TYPE_CHECKING, Any

import numpy as np
from fmpy import extract, read_model_description
from fmpy.fmi2 import FMU2Slave

from polaris.fmu import FmuError, compile_source_fmu, inspect_fmu
from polaris.result import Result

if TYPE_CHECKING:
    from polaris.backends import Backend
    from polaris.model import Model


class SessionError(FmuError):
    """The session was used in a state where the request is not possible."""


class InteractiveSession:
    """A running FMU that is advanced one communication step at a time.

    Unlike :func:`~polaris.fmu_runtime.run_fmu`, which runs to a fixed end time, a session
    lets the caller change inputs and tunable parameters between steps, read any variable
    at the current time, and rewind with :meth:`reset`. The dashboard and live plots are
    built on it.

    Example::

        with InteractiveSession.from_model(Model("MassSpringDamper", ["msd.mo"])) as s:
            s.advance(1.0)
            s.set(c=0.5)       # change a tunable parameter mid-run
            s.advance(1.0)
            s.history.plot()

    Args:
        fmu: Path to a Co-Simulation FMI 2.0 FMU. Source-code FMUs (e.g. from rumoca) are
            compiled first when ``compile_sources`` is true.
        step_size: Communication step in seconds of simulated time.
        substeps: Internal FMU steps per communication step. OpenModelica's FMUs use
            forward Euler, so more substeps give a more accurate trajectory at a higher cost.
        start_time: Simulation start time.
        parameters: Start values applied when the FMU is initialised.
        record: Variables kept in :attr:`history`; defaults to every time-varying variable.
        compile_sources: Compile source-code FMUs with the local C compiler.
    """

    def __init__(
        self,
        fmu: str | Path,
        *,
        step_size: float = 0.01,
        substeps: int = 1,
        start_time: float = 0.0,
        parameters: Mapping[str, float] | None = None,
        record: Sequence[str] | None = None,
        compile_sources: bool = True,
    ) -> None:
        if step_size <= 0:
            raise ValueError("step_size must be positive")
        if substeps < 1:
            raise ValueError("substeps must be at least 1")
        self.step_size = step_size
        self.substeps = substeps
        self.start_time = start_time
        self._start_values: dict[str, float] = dict(parameters or {})

        # Everything this session creates on disk lives under one directory, removed in close().
        self._scratch = Path(tempfile.mkdtemp(prefix="polaris_session_"))
        try:
            info = inspect_fmu(fmu)
            if not info.runnable and compile_sources and "c-code" in info.platforms:
                info = inspect_fmu(
                    compile_source_fmu(info.path, self._scratch / "compiled" / info.path.name)
                )
            if not info.runnable:
                raise SessionError(
                    f"{info.path.name} has no binary for this platform; "
                    "source-code FMUs need compile_sources=True and a C compiler"
                )
            self._description = read_model_description(str(info.path), validate=False)
            if self._description.coSimulation is None:
                raise SessionError(f"{info.path.name} does not support Co-Simulation")
            if not str(self._description.fmiVersion).startswith("2"):
                raise SessionError("Only FMI 2.0 FMUs are supported by InteractiveSession")
            self._unzipped = extract(str(info.path), str(self._scratch / "unzipped"))

            self._variables = {v.name: v for v in self._description.modelVariables}
            for name in [*self._start_values, *(record or ())]:
                self._require(name)
            self._recorded = list(record) if record else self._default_record()

            self._slave: FMU2Slave | None = None
            self._steps = 0
            self._started = False
            self._closed = False
            self._times: list[float] = []
            self._rows: list[list[float]] = []
            self._instantiate()
        except Exception:
            self._closed = True
            self._teardown()
            shutil.rmtree(self._scratch, ignore_errors=True)
            raise

    @classmethod
    def from_model(
        cls,
        model: Model,
        *,
        backend: str | Backend | None = None,
        fmu_dir: str | Path | None = None,
        **options: Any,
    ) -> InteractiveSession:
        """Export ``model`` as a Co-Simulation FMU with a backend and open a session on it.

        Args:
            model: The model to export.
            backend: Backend to export with; defaults to the model's own.
            fmu_dir: Where to keep the exported FMU. By default it is built in a
                temporary directory that is deleted once the session has loaded it.
            **options: Passed to the constructor (``step_size``, ``parameters`` ...).
        """
        # Model parameters become start values, so the session starts from the same state
        # as ``model.simulate()``; explicit ``parameters`` still win.
        options["parameters"] = {**model.parameters, **options.get("parameters", {})}
        if fmu_dir is not None:
            return cls(model.export_fmu(Path(fmu_dir), backend=backend), **options)
        with tempfile.TemporaryDirectory(prefix="polaris_fmu_") as tmp:
            fmu = model.export_fmu(Path(tmp), backend=backend)
            # The constructor unzips the FMU, so it may be deleted afterwards.
            return cls(fmu, **options)

    # -- state ---------------------------------------------------------------------------

    @property
    def time(self) -> float:
        """Current simulation time."""
        return self.start_time + self._steps * self.step_size

    @property
    def variables(self) -> tuple[str, ...]:
        """Names of every variable in the FMU."""
        return tuple(self._variables)

    @property
    def inputs(self) -> tuple[str, ...]:
        """Variables that are meant to be driven from outside (causality ``input``)."""
        return tuple(n for n, v in self._variables.items() if v.causality == "input")

    @property
    def tunable(self) -> tuple[str, ...]:
        """Parameters that can be changed while the simulation runs."""
        return tuple(
            n
            for n, v in self._variables.items()
            if v.causality == "parameter" and v.variability == "tunable"
        )

    @property
    def history(self) -> Result:
        """Everything recorded so far, as a :class:`~polaris.result.Result`."""
        self._ensure_started()
        data = np.array(self._rows, dtype=float).reshape(len(self._rows), len(self._recorded))
        variables = {name: data[:, i] for i, name in enumerate(self._recorded)}
        meta = {"backend": "fmu", "model": str(self._description.modelName), "mode": "interactive"}
        return Result(np.array(self._times, dtype=float), variables, meta)

    # -- control -------------------------------------------------------------------------

    def start(self) -> None:
        """Initialise the FMU. Called automatically by the first step or read."""
        self._check_open()
        if self._started:
            return
        slave = self._instance()
        slave.setupExperiment(startTime=self.start_time)
        slave.enterInitializationMode()
        slave.exitInitializationMode()
        self._started = True
        self._record()

    def step(self, count: int = 1) -> dict[str, float]:
        """Advance by ``count`` communication steps and return the new values.

        The returned dict holds the current ``time`` and every recorded variable.
        """
        if count < 1:
            raise ValueError("count must be at least 1")
        self._ensure_started()
        slave = self._instance()
        inner = self.step_size / self.substeps
        for _ in range(count):
            t0 = self.time
            for k in range(self.substeps):
                slave.doStep(currentCommunicationPoint=t0 + k * inner, communicationStepSize=inner)
            self._steps += 1
            self._record()
        return self.snapshot()

    def advance(
        self,
        duration: float,
        on_step: Callable[[InteractiveSession], None] | None = None,
    ) -> dict[str, float]:
        """Run for ``duration`` seconds of simulated time (rounded up to whole steps).

        Args:
            duration: Simulated time to cover.
            on_step: Called after every step, e.g. to feed back an input computed from
                the latest outputs.
        """
        if duration <= 0:
            raise ValueError("duration must be positive")
        # The small tolerance stops float noise (0.3 / 0.1 = 2.9999...) adding a step.
        count = int(np.ceil(duration / self.step_size - 1e-9))
        for _ in range(count):
            self.step()
            if on_step is not None:
                on_step(self)
        return self.snapshot()

    def set(
        self,
        values: Mapping[str, float] | None = None,
        /,
        *,
        force: bool = False,
        **named: float,
    ) -> None:
        """Change variable values, either before starting or between steps.

        Before the first step this sets start values, so any variable with a start value
        may be given. Once running only inputs and tunable parameters can change; the FMI
        standard forbids changing fixed parameters, so those need :meth:`reset` with new
        ``parameters``.

        Args:
            values: Name -> value mapping (an alternative to keyword arguments).
            force: Write fixed parameters mid-run anyway. Compilers such as OpenModelica
                declare every parameter fixed although their code reads it live, so this
                works for them. It is outside the standard, so the value is read back and
                :class:`SessionError` is raised if the FMU did not accept it. It cannot
                affect parameters the compiler already folded into constants.
            **named: Same as ``values``.
        """
        self._check_open()
        merged = {**(values or {}), **named}
        for name in merged:
            self._require(name)
        forced: list[str] = []
        if self._started:
            for name in merged:
                var = self._variables[name]
                if var.causality == "parameter" and var.variability != "tunable":
                    if not force:
                        raise SessionError(
                            f"'{name}' is a fixed parameter and cannot change while running; "
                            f"use reset(parameters={{'{name}': ...}}) or set(..., force=True)"
                        )
                    forced.append(name)
        else:
            # Remembered so a later reset() starts from the same values.
            self._start_values.update(merged)
        slave = self._instance()
        for name, value in merged.items():
            self._write(slave, name, value)
        for name in forced:
            if self._read(slave, name) != float(merged[name]):
                raise SessionError(f"The FMU did not accept the new value of '{name}'")

    def get(self, *names: str) -> dict[str, float]:
        """Current value of each named variable (all recorded variables if none given)."""
        self._ensure_started()
        wanted = names or tuple(self._recorded)
        for name in wanted:
            self._require(name)
        slave = self._instance()
        return {name: self._read(slave, name) for name in wanted}

    def snapshot(self) -> dict[str, float]:
        """Current time plus every recorded variable."""
        return {"time": self.time, **self.get()}

    def reset(self, parameters: Mapping[str, float] | None = None) -> None:
        """Return to the start time and clear the history.

        Start values from the constructor, from :meth:`set` before the first step and from
        ``parameters`` here are re-applied; changes made while running are discarded.
        """
        self._check_open()
        for name in parameters or {}:
            self._require(name)
        self._start_values.update(parameters or {})
        self._teardown()
        self._instantiate()

    def close(self) -> None:
        """Release the FMU and delete temporary files. Safe to call more than once."""
        if self._closed:
            return
        self._closed = True
        self._teardown()
        shutil.rmtree(self._scratch, ignore_errors=True)

    def __enter__(self) -> InteractiveSession:
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc: BaseException | None,
        tb: TracebackType | None,
    ) -> None:
        self.close()

    # -- internals -----------------------------------------------------------------------

    def _instantiate(self) -> None:
        self._steps = 0
        self._started = False
        self._times.clear()
        self._rows.clear()
        md = self._description
        slave = FMU2Slave(
            guid=md.guid,
            unzipDirectory=self._unzipped,
            modelIdentifier=md.coSimulation.modelIdentifier,
            instanceName="polaris",
        )
        slave.instantiate()
        self._slave = slave
        # Start values must be written while the instance is still uninitialised.
        for name, value in self._start_values.items():
            self._write(slave, name, value)

    def _teardown(self) -> None:
        slave, self._slave = getattr(self, "_slave", None), None
        if slave is None:
            return
        try:
            if self._started:
                slave.terminate()
        finally:
            # Always release the library, even if terminate failed.
            slave.freeInstance()

    def _instance(self) -> FMU2Slave:
        if self._slave is None:
            raise SessionError("Session is closed")
        return self._slave

    def _check_open(self) -> None:
        if self._closed:
            raise SessionError("Session is closed")

    def _ensure_started(self) -> None:
        self._check_open()
        if not self._started:
            self.start()

    def _require(self, name: str) -> None:
        if name not in self._variables:
            raise KeyError(f"Unknown FMU variable '{name}'")

    def _default_record(self) -> list[str]:
        # Some compilers (rumoca) expose "time" as a variable; Result already has a time axis.
        return [
            n
            for n, v in self._variables.items()
            if v.causality != "parameter"
            and v.variability != "fixed"
            and v.type != "String"
            and n != "time"
        ]

    def _record(self) -> None:
        slave = self._instance()
        self._times.append(self.time)
        self._rows.append([self._read(slave, name) for name in self._recorded])

    def _read(self, slave: FMU2Slave, name: str) -> float:
        var = self._variables[name]
        ref = [var.valueReference]
        if var.type == "Real":
            return float(slave.getReal(ref)[0])
        if var.type in ("Integer", "Enumeration"):
            return float(slave.getInteger(ref)[0])
        if var.type == "Boolean":
            return float(slave.getBoolean(ref)[0])
        raise SessionError(f"Cannot read '{name}' of type {var.type}")

    def _write(self, slave: FMU2Slave, name: str, value: float) -> None:
        var = self._variables[name]
        ref = [var.valueReference]
        if var.type == "Real":
            slave.setReal(ref, [float(value)])
        elif var.type in ("Integer", "Enumeration"):
            slave.setInteger(ref, [int(value)])
        elif var.type == "Boolean":
            slave.setBoolean(ref, [bool(value)])
        else:
            raise SessionError(f"Cannot set '{name}' of type {var.type}")
