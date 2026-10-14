from __future__ import annotations

import os
import shutil
from pathlib import Path

import pytest

PROJECT_ROOT = Path(__file__).resolve().parents[1]
PACKAGE_ROOT = PROJECT_ROOT / "ModelicaAutomotive"
PACKAGE_FILE = PACKAGE_ROOT / "package.mo"
PACKAGE_SOURCES = (
    PACKAGE_FILE,
    *sorted(path for path in PACKAGE_ROOT.rglob("*.mo") if path != PACKAGE_FILE),
)
BACKEND_EXECUTABLES = {"openmodelica": "omc", "rumoca": "rumoca"}


def _requested_backends() -> tuple[str, ...]:
    configured = os.environ.get("MODELICA_AUTOMOTIVE_BACKENDS")
    if configured is None:
        return tuple(BACKEND_EXECUTABLES)
    requested = tuple(name.strip() for name in configured.split(",") if name.strip())
    invalid = sorted(set(requested) - set(BACKEND_EXECUTABLES))
    if not requested or invalid:
        raise pytest.UsageError(
            "MODELICA_AUTOMOTIVE_BACKENDS must contain openmodelica and/or rumoca; "
            f"invalid={invalid}"
        )
    return requested


REQUESTED_BACKENDS = _requested_backends()


def library_model_files(backend: str) -> tuple[Path, ...]:
    if backend == "rumoca":
        return (PACKAGE_FILE, PACKAGE_ROOT.parent, *PACKAGE_SOURCES[1:])
    return (PACKAGE_FILE,)


@pytest.fixture(params=REQUESTED_BACKENDS)
def modelica_backend(request: pytest.FixtureRequest) -> str:
    executable = BACKEND_EXECUTABLES[request.param]
    if shutil.which(executable) is None:
        pytest.fail(
            f"required backend {request.param!r} is unavailable: "
            f"{executable!r} was not found on PATH"
        )
    return str(request.param)


@pytest.fixture
def openmodelica_backend() -> str:
    if shutil.which(BACKEND_EXECUTABLES["openmodelica"]) is None:
        pytest.fail(
            "this validation requires OpenModelica: "
            f"{BACKEND_EXECUTABLES['openmodelica']!r} was not found on PATH"
        )
    return "openmodelica"
