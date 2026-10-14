from __future__ import annotations

from conftest import PROJECT_ROOT


def test_every_example_model_has_a_python_runner():
    examples = PROJECT_ROOT / "ModelicaAerospace" / "Examples"
    model_names = {path.stem for path in examples.glob("*.mo") if path.name != "package.mo"}
    runner_names = {
        path.stem.removeprefix("run_") for path in (PROJECT_ROOT / "scripts").glob("run_*.py")
    }

    assert runner_names == model_names
