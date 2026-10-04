"""Run one FMU as a HELICS value federate."""

from __future__ import annotations

import importlib
import math
from collections.abc import Mapping
from pathlib import Path
from typing import Any

from polaris.fmu import inspect_fmu
from polaris.interactive.session import InteractiveSession
from polaris.result import Result


class FmuFederate:
    """Bind a Co-Simulation FMU's scalar inputs and outputs to HELICS values.

    The class owns both its :class:`InteractiveSession` and HELICS federate. One call to
    :meth:`run` enters HELICS execution mode and advances the FMU in fixed communication
    steps, applying subscribed inputs before each FMU step and publishing outputs after it.
    A HELICS broker and other federates must be created separately.

    ``inputs`` maps FMU input names to HELICS subscription keys. ``outputs`` maps FMU
    variable names to HELICS publication keys. Mappings are explicit to keep signal names
    and units under application control.

    Args:
        fmu: Runnable FMI 2.0 Co-Simulation FMU, or source FMU if a local C compiler exists.
        name: Unique HELICS federate name.
        inputs: FMU input name -> HELICS publication key to subscribe to.
        outputs: FMU variable name -> HELICS publication key to publish.
        step_size: HELICS period and FMU communication step, in simulation-time units.
        substeps: Number of FMU internal steps per HELICS time step.
        start_time: Simulation start time.
        parameters: FMU start-value overrides.
        broker: HELICS broker name or address; omit when using implicit broker discovery.
        core_type: HELICS core type, such as ``zmq`` or ``tcp``.
        core_init: Additional HELICS core initialization arguments.
        compile_sources: Compile source-code FMUs locally before loading.

    Raises:
        ImportError: HELICS bindings are not installed; install ``polaris[helics]``.
        ValueError: The FMU or mappings are incompatible with the requested federation.
    """

    def __init__(
        self,
        fmu: str | Path,
        name: str,
        *,
        inputs: Mapping[str, str] | None = None,
        outputs: Mapping[str, str] | None = None,
        step_size: float = 0.01,
        substeps: int = 1,
        start_time: float = 0.0,
        parameters: Mapping[str, float] | None = None,
        broker: str | None = None,
        core_type: str = "zmq",
        core_init: str = "",
        compile_sources: bool = True,
    ) -> None:
        if not name:
            raise ValueError("Federate name must not be empty")
        if not math.isfinite(step_size) or step_size <= 0:
            raise ValueError("step_size must be a positive finite number")
        if not math.isfinite(start_time):
            raise ValueError("start_time must be finite")
        if substeps < 1:
            raise ValueError("substeps must be at least 1")

        try:
            self._helics = importlib.import_module("helics")
        except ImportError as exc:
            raise ImportError(
                "HELICS support is optional; install it with `pip install polaris[helics]`."
            ) from exc

        self.name = name
        self.step_size = step_size
        self.start_time = start_time
        self.inputs = dict(inputs or {})
        self.outputs = dict(outputs or {})
        if not self.outputs:
            raise ValueError("At least one output mapping is required")
        if len(set(self.inputs.values())) != len(self.inputs):
            raise ValueError("Each FMU input must use a unique HELICS subscription key")
        if len(set(self.outputs.values())) != len(self.outputs):
            raise ValueError("Each FMU output must use a unique HELICS publication key")

        info = inspect_fmu(fmu)
        if not info.co_simulation:
            raise ValueError(f"{info.path.name} does not support FMI Co-Simulation")
        if not info.fmi_version.startswith("2"):
            raise ValueError("FmuFederate currently supports FMI 2.0 FMUs only")

        self._session: InteractiveSession | None = None
        self._federate: Any = None
        self._closed = False
        self._ran = False
        try:
            self._session = InteractiveSession(
                info.path,
                step_size=step_size,
                substeps=substeps,
                start_time=start_time,
                parameters=parameters,
                compile_sources=compile_sources,
            )
            self._validate_mappings()
            self._federate = self._create_federate(name, broker, core_type, core_init, step_size)
        except Exception:
            self.close()
            raise

    def _validate_mappings(self) -> None:
        """Reject non-real or incorrectly directed variables before HELICS registration."""
        assert self._session is not None
        variables = self._session._variables
        for variable, key in self.inputs.items():
            if not key:
                raise ValueError(f"HELICS subscription key for '{variable}' is empty")
            if variable not in variables:
                raise KeyError(f"Unknown FMU input variable '{variable}'")
            item = variables[variable]
            if item.causality != "input" or item.type != "Real":
                raise ValueError(f"'{variable}' must be an FMI Real input")
        for variable, key in self.outputs.items():
            if not key:
                raise ValueError(f"HELICS publication key for '{variable}' is empty")
            if variable not in variables:
                raise KeyError(f"Unknown FMU output variable '{variable}'")
            item = variables[variable]
            if item.causality not in ("output", "local") or item.type != "Real":
                raise ValueError(f"'{variable}' must be an FMI Real output or local variable")

    def _create_federate(
        self, name: str, broker: str | None, core_type: str, core_init: str, step_size: float
    ) -> Any:
        """Create the HELICS federate and register its publications and subscriptions."""
        helics = self._helics
        info = helics.helicsCreateFederateInfo()
        try:
            helics.helicsFederateInfoSetCoreTypeFromString(info, core_type)
            helics.helicsFederateInfoSetTimeProperty(
                info, helics.HELICS_PROPERTY_TIME_DELTA, step_size
            )
            if broker:
                helics.helicsFederateInfoSetBroker(info, broker)
            if core_init:
                helics.helicsFederateInfoSetCoreInitString(info, core_init)
            federate = helics.helicsCreateValueFederate(name, info)
        finally:
            helics.helicsFederateInfoFree(info)

        try:
            self._publications = {
                variable: helics.helicsFederateRegisterGlobalPublication(
                    federate, key, helics.HELICS_DATA_TYPE_DOUBLE, ""
                )
                for variable, key in self.outputs.items()
            }
            self._subscriptions = {
                variable: helics.helicsFederateRegisterSubscription(federate, key, "")
                for variable, key in self.inputs.items()
            }
        except Exception:
            helics.helicsFederateFree(federate)
            raise
        return federate

    def run(self, stop_time: float) -> Result:
        """Run this federate to ``stop_time`` and return the FMU trajectory.

        The stop time must be an integer number of communication steps from ``start_time``.
        HELICS time grants synchronize the FMU step; values from connected publications
        are applied at each grant before advancing the FMU.
        """
        if self._closed:
            raise RuntimeError("Federate is closed")
        if self._ran:
            raise RuntimeError("A federate can only be run once")
        if not math.isfinite(stop_time) or stop_time <= self.start_time:
            raise ValueError("stop_time must be finite and greater than start_time")
        steps_float = (stop_time - self.start_time) / self.step_size
        steps = round(steps_float)
        if not math.isclose(steps_float, steps, rel_tol=0, abs_tol=1e-9):
            raise ValueError("stop_time must align with the HELICS step_size")
        assert self._session is not None
        helics = self._helics
        self._ran = True
        try:
            self._session.start()
            helics.helicsFederateEnterExecutingMode(self._federate)
            self._publish_outputs()
            for index in range(1, steps + 1):
                target_time = self.start_time + index * self.step_size
                grant = helics.helicsFederateRequestTime(self._federate, target_time)
                if grant + 1e-9 < target_time:
                    raise RuntimeError(
                        f"HELICS granted t={grant:g} before requested t={target_time:g}"
                    )
                if self._subscriptions:
                    self._session.set(
                        {
                            name: float(helics.helicsInputGetDouble(subscription))
                            for name, subscription in self._subscriptions.items()
                        }
                    )
                self._session.step()
                self._publish_outputs()
            return self._session.history
        finally:
            # Let the broker release this federate even if an FMU or HELICS call fails.
            helics.helicsFederateDisconnect(self._federate)

    def _publish_outputs(self) -> None:
        """Publish current FMU values at the current HELICS time."""
        assert self._session is not None
        for variable, publication in self._publications.items():
            value = self._session.get(variable)[variable]
            self._helics.helicsPublicationPublishDouble(publication, value)

    def close(self) -> None:
        """Release HELICS resources and the wrapped FMU session; safe to call repeatedly."""
        if self._closed:
            return
        self._closed = True
        helics = getattr(self, "_helics", None)
        try:
            if helics is not None and self._federate is not None:
                try:
                    if not self._ran:
                        helics.helicsFederateDisconnect(self._federate)
                finally:
                    helics.helicsFederateFree(self._federate)
                    self._federate = None
        finally:
            if self._session is not None:
                self._session.close()
                self._session = None

    def __enter__(self) -> FmuFederate:
        """Use the federate as a context manager that owns its FMU and HELICS resources."""
        if self._closed:
            raise RuntimeError("Federate is closed")
        return self

    def __exit__(self, exc_type: Any, exc: Any, tb: Any) -> None:
        """Finalize and release the federate on context exit."""
        self.close()
