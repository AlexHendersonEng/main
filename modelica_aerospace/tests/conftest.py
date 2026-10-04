from __future__ import annotations

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


def library_model_files(backend: str) -> tuple[Path, ...]:
    if backend == "rumoca":
        return (PACKAGE_FILE, PACKAGE_ROOT.parent, *PACKAGE_SOURCES[1:])
    return (PACKAGE_FILE,)


@pytest.fixture(params=("openmodelica", "rumoca"))
def modelica_backend(request: pytest.FixtureRequest) -> str:
    executable = {"openmodelica": "omc", "rumoca": "rumoca"}[request.param]
    if shutil.which(executable) is None:
        pytest.skip(f"{executable} not installed")
    return str(request.param)
