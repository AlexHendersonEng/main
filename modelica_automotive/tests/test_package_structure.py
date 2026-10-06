from __future__ import annotations

import importlib.util
import re
import tomllib

from conftest import PACKAGE_ROOT, PROJECT_ROOT


def _load_checker():
    path = PROJECT_ROOT / "scripts" / "check_package.py"
    spec = importlib.util.spec_from_file_location("check_modelica_automotive", path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_package_tree_is_complete_and_ordered():
    checker = _load_checker()
    assert checker.validate_package(PACKAGE_ROOT) == []


def test_expected_public_namespaces_exist():
    expected = {
        "Aerodynamics",
        "Brakes",
        "Constants",
        "Control",
        "Drivers",
        "Examples",
        "Interfaces",
        "Mathematics",
        "Powertrain",
        "Road",
        "Scenarios",
        "Sensors",
        "Steering",
        "Suspension",
        "Tests",
        "Tires",
        "Types",
        "Utilities",
        "VehicleDynamics",
        "Wheels",
    }
    actual = {
        path.name
        for path in PACKAGE_ROOT.iterdir()
        if path.is_dir() and (path / "package.mo").is_file()
    }
    assert actual == expected


def test_all_modelica_sources_are_inside_the_library_package():
    sources = set(PROJECT_ROOT.rglob("*.mo"))
    assert sources
    assert all(PACKAGE_ROOT in path.parents for path in sources)


def test_compiler_validation_models_are_packaged_by_domain():
    tests_package = PACKAGE_ROOT / "Tests"
    domains = {
        path.name
        for path in tests_package.iterdir()
        if path.is_dir() and (path / "package.mo").is_file()
    }
    assert domains == {
        "Common",
        "Longitudinal",
        "MathematicsRoad",
        "Planar",
        "PowertrainAerodynamics",
        "DriversSensorsControl",
        "Scenarios",
        "SuspensionRigidBody",
        "Tires",
        "WheelsBrakes",
    }
    assert not list((PROJECT_ROOT / "tests").rglob("*.mo"))


def test_repository_metadata_files_exist():
    for name in (
        ".gitignore",
        "README.md",
        "TRACEABILITY.md",
        "LICENSE",
        "pyproject.toml",
        "uv.lock",
    ):
        path = PROJECT_ROOT / name
        assert path.is_file() and path.stat().st_size > 0


def test_every_public_executable_class_is_traceable():
    traceability = (PROJECT_ROOT / "TRACEABILITY.md").read_text(encoding="utf-8")
    declarations = re.compile(r"^(?:block|function|model)\s+([A-Za-z_][A-Za-z0-9_]*)", re.MULTILINE)
    public_classes: set[str] = set()
    for source in PACKAGE_ROOT.rglob("*.mo"):
        relative = source.relative_to(PACKAGE_ROOT)
        if relative.name == "package.mo" or relative.parts[0] == "Tests":
            continue
        match = declarations.search(source.read_text(encoding="utf-8"))
        if match:
            namespace = ".".join(relative.parts[:-1])
            prefix = f"ModelicaAutomotive.{namespace}" if namespace else "ModelicaAutomotive"
            public_classes.add(f"{prefix}.{match.group(1)}")

    missing = sorted(name for name in public_classes if f"`{name}`" not in traceability)
    assert not missing, f"public executable classes missing from TRACEABILITY.md: {missing}"


def test_public_executable_classes_have_short_descriptions():
    declarations = re.compile(
        r"^(?:block|function|model)\s+[A-Za-z_][A-Za-z0-9_]*\s+\"[^\"]+\"",
        re.MULTILINE,
    )
    missing = []
    for source in PACKAGE_ROOT.rglob("*.mo"):
        relative = source.relative_to(PACKAGE_ROOT)
        if relative.name == "package.mo" or relative.parts[0] == "Tests":
            continue
        text = source.read_text(encoding="utf-8")
        if re.search(r"^(?:block|function|model)\s+", text, re.MULTILINE):
            if not declarations.search(text):
                missing.append(str(relative))
    assert not missing, f"public executable classes missing descriptions: {missing}"


def test_examples_are_bounded_and_documented():
    examples = PACKAGE_ROOT / "Examples"
    for source in examples.glob("*.mo"):
        if source.name == "package.mo":
            continue
        text = source.read_text(encoding="utf-8")
        assert "experiment(" in text, f"{source.name} lacks an experiment annotation"
        stop_time = re.search(r"StopTime\s*=\s*([0-9]+(?:\.[0-9]+)?)", text)
        assert stop_time is not None, f"{source.name} lacks a numeric StopTime"
        assert float(stop_time.group(1)) > 0, f"{source.name} has a non-positive StopTime"
        assert "Documentation(info=" in text, f"{source.name} lacks documentation"


def test_release_documentation_covers_usage_and_limits():
    readme = (PROJECT_ROOT / "README.md").read_text(encoding="utf-8")
    for heading in (
        "## Package map",
        "## Model selection",
        "## Validity limits and unsupported workflows",
        "## MSL reuse and backend compatibility",
        "## Extension points",
        "## Validation",
    ):
        assert heading in readme
    assert "No continuous-integration workflow is included" in readme
    assert "Modelica Standard Library 4.0.0" in readme


def test_python_tooling_is_managed_by_uv():
    config = tomllib.loads((PROJECT_ROOT / "pyproject.toml").read_text(encoding="utf-8"))
    assert config["project"]["name"] == "modelica-automotive"
    assert config["project"]["requires-python"] == ">=3.14"
    assert config["project"]["dependencies"] == ["polaris"]
    assert config["tool"]["uv"]["sources"]["polaris"] == {
        "git": "https://github.com/AlexHendersonEng/main.git",
        "rev": "ccb9e096d9d070d19552abdaa2b7f3fefe161c87",
        "subdirectory": "polaris",
    }
    assert {"pytest", "ruff"} <= set(config["dependency-groups"]["dev"])
