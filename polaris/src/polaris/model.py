"""User-facing model description and the ``simulate`` entry point."""

from __future__ import annotations

import shutil
import tempfile
from dataclasses import dataclass, field, replace
from pathlib import Path

from polaris.backends import Backend, Capability, available_backends, get_backend
from polaris.result import Result
from polaris.types import FmuKind, ModelSource, SimulationOptions


def resolve_backend(backend: str | Backend | None) -> Backend:
    """Turn a name, instance or ``None`` into a usable backend.

    ``None`` picks the first backend whose compiler is installed, in registration order.
    """
    if isinstance(backend, Backend):
        return backend
    if backend is not None:
        return get_backend(backend)
    names = available_backends()
    if not names:
        raise RuntimeError("No Modelica backend is available; install OpenModelica or rumoca")
    return get_backend(names[0])


@dataclass(frozen=True)
class Model:
    """A Modelica class to build and run.

    Args:
        class_name: Fully qualified class to simulate.
        files: Modelica files that define it (and anything it depends on).
        libraries: Libraries to load, e.g. ``("Modelica",)``.
        parameters: Default parameter overrides, replaced per call via ``simulate(parameters=...)``.
        backend: Default backend (name or instance); ``None`` auto-selects.
    """

    class_name: str
    files: tuple[Path, ...] = ()
    libraries: tuple[str, ...] = ()
    parameters: dict[str, float] = field(default_factory=dict)
    backend: str | Backend | None = None

    def __post_init__(self) -> None:
        # Accept str paths and lists for convenience, but store immutable Paths.
        object.__setattr__(self, "files", tuple(Path(f) for f in self.files))
        object.__setattr__(self, "libraries", tuple(self.libraries))

    def source(self, parameters: dict[str, float] | None = None) -> ModelSource:
        """Backend-facing description, with ``parameters`` layered over the defaults."""
        return ModelSource(
            self.class_name,
            self.files,
            self.libraries,
            {**self.parameters, **(parameters or {})},
        )

    def simulate(
        self,
        options: SimulationOptions | None = None,
        *,
        parameters: dict[str, float] | None = None,
        backend: str | Backend | None = None,
        work_dir: str | Path | None = None,
    ) -> Result:
        """Compile and run the model.

        Args:
            options: Solver settings; defaults to ``SimulationOptions()``.
            parameters: Parameter overrides for this run only.
            backend: Overrides the model's default backend.
            work_dir: Where build artefacts go. By default a temporary directory is
                used and removed afterwards (the Result is fully loaded into memory).
        """
        opts = options or SimulationOptions()
        be = resolve_backend(backend or self.backend)
        if not be.supports(Capability.SIMULATE):
            raise NotImplementedError(f"Backend '{be.name}' cannot simulate")
        source = self.source(parameters)
        meta = {"backend": be.name, "model": self.class_name, "version": str(be.version())}

        if work_dir is not None:
            return Result.from_csv(be.simulate(source, opts, Path(work_dir)), meta)
        with tempfile.TemporaryDirectory(prefix="polaris_") as tmp:
            return Result.from_csv(be.simulate(source, opts, Path(tmp)), meta)

    def export_fmu(
        self,
        destination: str | Path,
        *,
        kind: FmuKind = FmuKind.CO_SIMULATION,
        fmi_version: str = "2.0",
        backend: str | Backend | None = None,
        work_dir: str | Path | None = None,
    ) -> Path:
        """Export an FMU and copy it to ``destination`` (a file path or existing directory)."""
        be = resolve_backend(backend or self.backend)
        if not be.supports(Capability.EXPORT_FMU):
            raise NotImplementedError(f"Backend '{be.name}' cannot export FMUs")
        dest = Path(destination)
        if dest.is_dir():
            dest = dest / f"{self.class_name}.fmu"

        def run(directory: Path) -> Path:
            built = be.export_fmu(self.source(), kind, fmi_version, directory)
            dest.parent.mkdir(parents=True, exist_ok=True)
            return Path(shutil.copyfile(built, dest))

        if work_dir is not None:
            return run(Path(work_dir))
        with tempfile.TemporaryDirectory(prefix="polaris_") as tmp:
            return run(Path(tmp))

    def with_parameters(self, **values: float) -> Model:
        """Copy of this model with extra default parameter overrides."""
        return replace(self, parameters={**self.parameters, **values})
