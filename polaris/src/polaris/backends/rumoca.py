"""Rumoca backend.

Drives the ``rumoca`` command line tool. Rumoca simulates directly to CSV and can
generate FMUs through its ``fmi2``/``fmi3`` code-generation targets.

Known differences from OpenModelica (verified against rumoca 0.10.0):

* Exported FMUs are *source-code* FMUs (ME + CS in one archive, C sources and no
  prebuilt binaries), so they must be compiled by the importing tool.
* There is no run-time parameter override flag, so overrides are applied by
  compiling a small wrapper model that ``extends`` the requested class.
* Libraries are resolved through the ``MODELICAPATH`` environment variable.
"""

from __future__ import annotations

import json
import re
import shutil
import subprocess
from collections.abc import Mapping
from pathlib import Path
from typing import ClassVar

import numpy as np

from polaris.backends.base import (
    Backend,
    BackendError,
    BackendUnavailableError,
    Capability,
)
from polaris.backends.versioning import Version
from polaris.types import FmuKind, Jacobian, ModelSource, SimulationOptions

# Same rule as the OpenModelica backend: only plain Modelica paths may be spliced into
# generated source text.
_IDENT = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*(\.[A-Za-z_][A-Za-z0-9_]*)*$")

# Name of the generated wrapper class. It is distinct from any user class on purpose, since
# a wrapper with the same name as the class it extends would be self-referential.
_WRAPPER = "PolarisRun"

# FMI version string -> rumoca code-generation target.
_FMI_TARGETS = {"2.0": "fmi2", "3.0": "fmi3"}


def _check_ident(name: str, what: str) -> str:
    if not _IDENT.match(name):
        raise BackendError(f"Invalid {what} for Modelica: {name!r}")
    return name


class RumocaBackend(Backend):
    """Drives the ``rumoca`` executable.

    Args:
        executable: Path to rumoca. Defaults to the one on PATH.
    """

    name: ClassVar[str] = "rumoca"
    capabilities: ClassVar[Capability] = (
        Capability.SIMULATE | Capability.EXPORT_FMU | Capability.JACOBIAN
    )

    def __init__(self, executable: str | None = None) -> None:
        self._executable = executable or shutil.which("rumoca")
        self._version: Version | None = None

    # Cheap check: only looks for the executable, it does not start rumoca.
    def is_available(self) -> bool:
        return self._executable is not None

    def version(self) -> Version:
        if self._version is None:
            proc = self._run(["--version"], cwd=None)
            self._version = Version.parse(proc.stdout + proc.stderr)
        return self._version

    def simulate(self, source: ModelSource, options: SimulationOptions, work_dir: Path) -> Path:
        work_dir.mkdir(parents=True, exist_ok=True)
        entry, model, roots = self._entry(source, work_dir, with_parameters=True)
        result = work_dir / f"{source.class_name}_res.csv"
        args = ["sim", str(entry), "-m", model, "--t-end", repr(options.stop_time)]
        args += self._root_args(roots)
        if options.start_time != 0.0:
            # Rumoca always starts at t=0; silently shifting the window would mislead callers.
            raise BackendError("The rumoca backend only supports start_time = 0")
        if options.step_size:
            args += ["--dt", repr(options.step_size)]
        if options.solver:
            args += ["--solver", options.solver]
        # Rumoca has separate absolute/relative tolerances; use the one value for both.
        args += ["--atol", repr(options.tolerance), "--rtol", repr(options.tolerance)]
        args += ["-o", str(result)]
        self._run_checked(args, work_dir, result, "simulation")
        if options.outputs:
            _filter_columns(result, options.outputs)
        return result

    def export_fmu(
        self, source: ModelSource, kind: FmuKind, fmi_version: str, work_dir: Path
    ) -> Path:
        # Rumoca always produces a combined ME+CS archive, so ``kind`` is accepted for
        # interface compatibility but does not change the output.
        try:
            target = _FMI_TARGETS[fmi_version]
        except KeyError:
            supported = ", ".join(_FMI_TARGETS)
            raise BackendError(
                f"rumoca cannot export FMI {fmi_version}; supported: {supported}"
            ) from None
        work_dir.mkdir(parents=True, exist_ok=True)
        # Parameters are not baked in: FMUs expose them for the importer to set.
        entry, model, roots = self._entry(source, work_dir, with_parameters=False)
        out_dir = work_dir / f"{source.class_name}_fmu_{target}"
        args = ["compile", str(entry), "-m", model, "--target", target, "-o", str(out_dir)]
        args += self._root_args(roots)
        # The archive is named after the compiled model, which is the last name segment.
        produced = out_dir / f"{model.rsplit('.', 1)[-1]}.fmu"
        self._run_checked(args, work_dir, produced, "FMU export")
        final = work_dir / f"{source.class_name}.fmu"
        shutil.copyfile(produced, final)
        return final

    def jacobian(
        self,
        source: ModelSource,
        work_dir: Path,
        at: Mapping[str, float] | None = None,
    ) -> Jacobian:
        # ``--inspect jacobian`` differentiates the lowered model and prints dense matrices
        # as JSON, so no simulation is needed.
        work_dir.mkdir(parents=True, exist_ok=True)
        entry, model, roots = self._entry(source, work_dir, with_parameters=True)
        args = ["compile", str(entry), "-m", model, "--inspect", "jacobian", "--format", "json"]
        args += self._root_args(roots)
        if at:
            point = ",".join(f"{_check_ident(k, 'state')}={float(v)!r}" for k, v in at.items())
            args += ["--at", point]
        proc = self._run(args, work_dir)
        if proc.returncode != 0:
            raise BackendError(f"rumoca jacobian failed:\n{proc.stdout}{proc.stderr}")
        try:
            data = json.loads(proc.stdout)
        except json.JSONDecodeError as exc:
            raise BackendError(f"Unreadable rumoca jacobian output:\n{proc.stdout}") from exc
        # Each block carries its own ``error`` field (e.g. for models without states).
        for block in ("state_jacobian", "parameter_jacobian"):
            if data[block].get("error"):
                raise BackendError(f"rumoca {block}: {data[block]['error']}")
        state, param = data["state_jacobian"], data["parameter_jacobian"]
        return Jacobian(
            states=tuple(state["labels"]),
            state_matrix=np.array(state["matrix"], dtype=float),
            parameters=tuple(param["param_labels"]),
            parameter_matrix=np.array(param["matrix"], dtype=float),
            time=float(data["t"]),
            state_values={s["name"]: float(s["value"]) for s in data["states"]},
        )

    def _entry(
        self, source: ModelSource, work_dir: Path, *, with_parameters: bool
    ) -> tuple[Path, str, list[Path]]:
        """Pick rumoca's entry file, model name and extra source roots."""
        _check_ident(source.class_name, "class name")
        if not source.files:
            raise BackendError("The rumoca backend needs at least one Modelica file")
        files = [f.resolve() for f in source.files]
        if not (with_parameters and source.parameters):
            return files[0], source.class_name, files[1:]
        overrides = ", ".join(
            f"{_check_ident(k, 'parameter')} = {float(v)!r}" for k, v in source.parameters.items()
        )
        wrapper = work_dir / f"{_WRAPPER}.mo"
        wrapper.write_text(
            f"model {_WRAPPER}\n  extends {source.class_name}({overrides});\nend {_WRAPPER};\n",
            encoding="utf-8",
        )
        # The original files become source roots so the wrapper can find the class.
        return wrapper, _WRAPPER, files

    @staticmethod
    def _root_args(roots: list[Path]) -> list[str]:
        args: list[str] = []
        for root in roots:
            args += ["--source-root", str(root)]
        return args

    def _run(self, args: list[str], cwd: Path | None) -> subprocess.CompletedProcess[str]:
        if self._executable is None:
            raise BackendUnavailableError("rumoca was not found on PATH")
        return subprocess.run(
            [self._executable, *args], cwd=cwd, capture_output=True, text=True, check=False
        )

    def _run_checked(self, args: list[str], cwd: Path, expected: Path, what: str) -> None:
        proc = self._run(args, cwd)
        # Unlike omc, rumoca reports failures with a non-zero exit code; also require the
        # output so a silent no-op is never mistaken for success.
        if proc.returncode != 0 or not expected.exists():
            raise BackendError(f"rumoca {what} failed:\n{proc.stdout}{proc.stderr}")


def _filter_columns(csv_path: Path, names: tuple[str, ...]) -> None:
    """Keep only ``time`` and the requested columns (rumoca has no output filter flag)."""
    lines = csv_path.read_text(encoding="utf-8").splitlines()
    header = lines[0].split(",")
    keep = [i for i, col in enumerate(header) if col == "time" or col in names]
    csv_path.write_text(
        "\n".join(",".join(row.split(",")[i] for i in keep) for row in lines) + "\n",
        encoding="utf-8",
    )
