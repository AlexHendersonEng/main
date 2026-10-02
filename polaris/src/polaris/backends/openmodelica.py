"""OpenModelica backend.

Commands are issued to ``omc`` through OMPython when it is usable, otherwise by
running a ``.mos`` script with the ``omc`` executable. Operations never rely on
parsed return values: success is judged by the expected output file existing, and
the compiler log is attached to any error.
"""

from __future__ import annotations

import contextlib
import re
import shutil
import subprocess
from pathlib import Path
from typing import ClassVar, Protocol

from polaris.backends.base import (
    Backend,
    BackendError,
    BackendUnavailableError,
    Capability,
)
from polaris.backends.versioning import Version
from polaris.types import FmuKind, ModelSource, SimulationOptions

# Modelica class/parameter paths such as "Pkg.Model" or "sub.k". Anything else is rejected
# before it can be spliced into an omc command, which prevents command injection.
_IDENT = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*(\.[A-Za-z_][A-Za-z0-9_]*)*$")


class _Runner(Protocol):
    """Executes omc commands in ``cwd`` and returns the combined log text."""

    def run(self, commands: list[str], cwd: Path) -> str: ...


# Modelica string literal; backslashes become slashes so Windows paths survive omc parsing.
def _quote(text: str | Path) -> str:
    value = str(text).replace("\\", "/").replace('"', '\\"')
    return f'"{value}"'


def _check_ident(name: str, what: str) -> str:
    if not _IDENT.match(name):
        raise BackendError(f"Invalid {what} for Modelica: {name!r}")
    return name


class _CliRunner:
    """Runs commands by writing a .mos script and invoking the omc executable."""

    def __init__(self, executable: str) -> None:
        self._exe = executable

    def run(self, commands: list[str], cwd: Path) -> str:
        # omc resolves relative paths against its working directory, so cd there first.
        script = cwd / "polaris_run.mos"
        lines = [f"cd({_quote(cwd)});", *commands]
        script.write_text("\n".join(lines) + "\n", encoding="utf-8")
        proc = subprocess.run(
            [self._exe, script.name],
            cwd=cwd,
            capture_output=True,
            text=True,
            check=False,
        )
        # check=False: omc exits 0 even on model errors; callers check for output files.
        return proc.stdout + proc.stderr


class _OMPythonRunner:
    """Runs commands in a short-lived OMPython session (one omc process per call)."""

    def run(self, commands: list[str], cwd: Path) -> str:
        # Imported lazily so the backend still works when OMPython is missing.
        from OMPython import OMCSessionLocal

        session = OMCSessionLocal()
        out: list[str] = []
        try:
            for command in [f"cd({_quote(cwd)})", *commands]:
                try:
                    out.append(str(session.sendExpression(command.rstrip(";"))))
                except Exception as exc:
                    raise BackendError(f"OpenModelica command failed: {command}\n{exc}") from exc
        finally:
            # Always stop the omc process, even if a command failed.
            with contextlib.suppress(Exception):
                session.sendExpression("quit()", parsed=False)
        return "\n".join(out)


# OMPython is a hard dependency, but guard the import so a broken install degrades to the CLI.
def _ompython_usable() -> bool:
    try:
        import OMPython  # noqa: F401
    except ImportError:
        return False
    return True


class OpenModelicaBackend(Backend):
    """Drives OpenModelica (``omc``) to simulate models and export FMUs.

    Args:
        executable: Path to omc. Defaults to the one on PATH.
        use_ompython: Prefer OMPython over the CLI when possible.
    """

    name: ClassVar[str] = "openmodelica"
    capabilities: ClassVar[Capability] = Capability.SIMULATE | Capability.EXPORT_FMU

    def __init__(self, executable: str | None = None, *, use_ompython: bool = True) -> None:
        self._executable = executable or shutil.which("omc")
        # OMPython locates omc itself, so an explicit executable forces the CLI path.
        self._use_ompython = use_ompython and executable is None and _ompython_usable()
        self._version: Version | None = None

    # Cheap check: only looks for the executable, it does not start omc.
    def is_available(self) -> bool:
        return self._executable is not None

    def version(self) -> Version:
        if self._version is None:
            if self._executable is None:
                raise BackendUnavailableError("omc was not found on PATH")
            # Cached: version() is called often and spawning omc is slow.
            proc = subprocess.run(
                [self._executable, "--version"], capture_output=True, text=True, check=False
            )
            self._version = Version.parse(proc.stdout + proc.stderr)
        return self._version

    def simulate(self, source: ModelSource, options: SimulationOptions, work_dir: Path) -> Path:
        cls = _check_ident(source.class_name, "class name")
        # omc writes <Class>_res.csv into the working directory.
        call = self._simulate_call("simulate", cls, options, source)
        log = self._run([*self._load(source), call], work_dir)
        result = work_dir / f"{cls}_res.csv"
        self._require(result, "simulation", log)
        return result

    def export_fmu(
        self, source: ModelSource, kind: FmuKind, fmi_version: str, work_dir: Path
    ) -> Path:
        cls = _check_ident(source.class_name, "class name")
        if not re.fullmatch(r"\d+\.\d+", fmi_version):
            raise BackendError(f"Invalid FMI version: {fmi_version!r}")
        # FMI version and kind come from enums/regex-checked strings, so f-string use is safe.
        call = f'buildModelFMU({cls}, version="{fmi_version}", fmuType="{kind.value}");'
        log = self._run([*self._load(source), call], work_dir)
        fmu = work_dir / f"{cls}.fmu"
        self._require(fmu, "FMU export", log)
        return fmu

    def _run(self, commands: list[str], work_dir: Path) -> str:
        if self._executable is None:
            raise BackendUnavailableError("omc was not found on PATH")
        work_dir.mkdir(parents=True, exist_ok=True)
        runner: _Runner = _OMPythonRunner() if self._use_ompython else _CliRunner(self._executable)
        # Append getErrorString() so compiler diagnostics appear in the log on failure.
        return runner.run([*commands, "getErrorString();"], work_dir)

    # Errors are detected by the missing output file rather than by parsing omc results,
    # whose format varies between versions.
    @staticmethod
    def _require(path: Path, what: str, log: str) -> None:
        if not path.exists():
            raise BackendError(f"OpenModelica {what} failed; expected {path.name}.\n{log}")

    @staticmethod
    def _load(source: ModelSource) -> list[str]:
        # Libraries first so user files can depend on them.
        commands = [f"loadModel({_check_ident(lib, 'library')});" for lib in source.libraries]
        commands += [f"loadFile({_quote(f.resolve())});" for f in source.files]
        return commands

    @staticmethod
    def _simulate_call(
        function: str, cls: str, options: SimulationOptions, source: ModelSource
    ) -> str:
        """Build an omc call such as ``simulate(Model, startTime=...)``."""
        args = [
            f"startTime={options.start_time!r}",
            f"stopTime={options.stop_time!r}",
            f"tolerance={options.tolerance!r}",
            'outputFormat="csv"',
        ]
        if options.step_size:
            # omc takes an interval count, not a step size.
            intervals = max(1, round((options.stop_time - options.start_time) / options.step_size))
            args.append(f"numberOfIntervals={intervals}")
        if options.solver:
            args.append(f"method={_quote(options.solver)}")
        if options.outputs:
            # variableFilter is a regex, so escape names and OR them together.
            pattern = "|".join(re.escape(o) for o in options.outputs)
            args.append(f"variableFilter={_quote(pattern)}")
        if source.parameters:
            # Parameters are overridden at run time, so no recompilation per value.
            overrides = ",".join(
                f"{_check_ident(k, 'parameter')}={float(v)!r}" for k, v in source.parameters.items()
            )
            args.append(f"simflags={_quote('-override=' + overrides)}")
        return f"{function}({cls}, {', '.join(args)});"
