from __future__ import annotations

import importlib.util
import re
import tomllib

from conftest import PACKAGE_ROOT, PROJECT_ROOT


def _load_checker():
    path = PROJECT_ROOT / "scripts" / "check_package.py"
    spec = importlib.util.spec_from_file_location("check_modelica_aerospace", path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_package_tree_is_complete_and_ordered():
    checker = _load_checker()
    assert checker.validate_package(PACKAGE_ROOT) == []


def test_expected_public_namespaces_exist():
    expected = {
        "Actuators",
        "Aerodynamics",
        "Constants",
        "Control",
        "Coordinates",
        "Environment",
        "Examples",
        "FlightDynamics",
        "Guidance",
        "Interfaces",
        "Mathematics",
        "Navigation",
        "Propulsion",
        "Sensors",
        "Tests",
        "Types",
        "Utilities",
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
        "Coordinates",
        "Environment",
        "FlightDynamics",
        "Mathematics",
        "Subsystems",
        "GuidanceNavigationControl",
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
            prefix = f"ModelicaAerospace.{namespace}" if namespace else "ModelicaAerospace"
            public_classes.add(f"{prefix}.{match.group(1)}")

    missing = sorted(name for name in public_classes if f"`{name}`" not in traceability)
    assert not missing, f"public executable classes missing from TRACEABILITY.md: {missing}"


def test_ci_workflow_exists():
    workflow = PROJECT_ROOT.parent / ".github" / "workflows" / "modelica-aerospace.yml"
    assert workflow.is_file() and workflow.stat().st_size > 0


def test_python_tooling_is_managed_by_uv():
    config = tomllib.loads((PROJECT_ROOT / "pyproject.toml").read_text(encoding="utf-8"))

    assert config["project"]["name"] == "modelica-aerospace"
    assert config["project"]["requires-python"] == ">=3.14"
    assert config["project"]["dependencies"] == ["polaris"]
    assert config["tool"]["uv"]["sources"]["polaris"] == {
        "git": "https://github.com/AlexHendersonEng/main.git",
        "rev": "ccb9e096d9d070d19552abdaa2b7f3fefe161c87",
        "subdirectory": "polaris",
    }
    assert {"pytest", "ruff"} <= set(config["dependency-groups"]["dev"])

    lock = tomllib.loads((PROJECT_ROOT / "uv.lock").read_text(encoding="utf-8"))
    polaris = next(package for package in lock["package"] if package["name"] == "polaris")
    assert polaris["source"]["git"] == (
        "https://github.com/AlexHendersonEng/main.git"
        "?subdirectory=polaris&rev=ccb9e096d9d070d19552abdaa2b7f3fefe161c87"
        "#ccb9e096d9d070d19552abdaa2b7f3fefe161c87"
    )
