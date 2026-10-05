from __future__ import annotations

import os
import shutil
from pathlib import Path

import pytest

PROJECT_ROOT = Path(__file__).resolve().parents[1]
PACKAGE_ROOT = PROJECT_ROOT / "ModelicaAerospace"
PACKAGE_FILE = PACKAGE_ROOT / "package.mo"
PACKAGE_SOURCES = (
    PACKAGE_FILE,
    *sorted(path for path in PACKAGE_ROOT.rglob("*.mo") if path != PACKAGE_FILE),
)
BACKEND_EXECUTABLES = {"openmodelica": "omc", "rumoca": "rumoca"}


def _requested_backends() -> tuple[str, ...]:
    configured = os.environ.get("MODELICA_AEROSPACE_BACKENDS")
    if configured is None:
        return tuple(BACKEND_EXECUTABLES)
    requested = tuple(name.strip() for name in configured.split(",") if name.strip())
    invalid = sorted(set(requested) - set(BACKEND_EXECUTABLES))
    if not requested or invalid:
        raise pytest.UsageError(
            "MODELICA_AEROSPACE_BACKENDS must contain openmodelica and/or rumoca; "
            f"invalid={invalid}"
        )
    return requested


REQUESTED_BACKENDS = _requested_backends()
STRICT_BACKEND_SELECTION = "MODELICA_AEROSPACE_BACKENDS" in os.environ


def library_model_files(backend: str) -> tuple[Path, ...]:
    if backend == "rumoca":
        return (PACKAGE_FILE, PACKAGE_ROOT.parent, *PACKAGE_SOURCES[1:])
    return (PACKAGE_FILE,)


@pytest.fixture(params=REQUESTED_BACKENDS)
def modelica_backend(request: pytest.FixtureRequest) -> str:
    executable = BACKEND_EXECUTABLES[request.param]
    if shutil.which(executable) is None:
        if STRICT_BACKEND_SELECTION:
            pytest.fail(
                f"required backend {request.param!r} is unavailable: "
                f"{executable!r} was not found on PATH"
            )
        pytest.skip(f"{executable} not installed")
    return str(request.param)
