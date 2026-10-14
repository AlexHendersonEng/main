from __future__ import annotations

import re

from conftest import PACKAGE_ROOT, PROJECT_ROOT


def _script_name(example_name: str) -> str:
    if example_name.endswith("ABS"):
        example_name = f"{example_name[:-3]}Abs"
    name = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", example_name)
    return re.sub(r"(?<!^)(?=[A-Z])", "_", name).lower()


def test_every_example_model_has_a_python_runner():
    examples = PACKAGE_ROOT / "Examples"
    model_names = {path.stem for path in examples.glob("*.mo") if path.name != "package.mo"}
    runner_names = {
        path.stem
        for path in (PROJECT_ROOT / "scripts").glob("*.py")
        if path.name != "_example_runner.py" and path.name != "check_package.py"
    }

    assert runner_names == {_script_name(name) for name in model_names}
