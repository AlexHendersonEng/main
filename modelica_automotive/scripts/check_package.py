"""Validate Modelica package ordering and dependency metadata."""

from __future__ import annotations

import argparse
import re
from pathlib import Path

PACKAGE_NAME = "ModelicaAutomotive"


def _ordered_entries(package_dir: Path) -> list[str]:
    return [
        line.strip()
        for line in (package_dir / "package.order").read_text(encoding="utf-8").splitlines()
        if line.strip()
    ]


def _declared_entries(package_dir: Path) -> set[str]:
    entries = {path.stem for path in package_dir.glob("*.mo") if path.name != "package.mo"}
    entries.update(
        path.name
        for path in package_dir.iterdir()
        if path.is_dir() and (path / "package.mo").is_file()
    )
    package_source = (package_dir / "package.mo").read_text(encoding="utf-8")
    entries.update(
        re.findall(
            r"^  constant\s+[A-Za-z_][A-Za-z0-9_.]*\s+([A-Za-z_][A-Za-z0-9_]*)",
            package_source,
            flags=re.MULTILINE,
        )
    )
    return entries


def validate_package(package_root: Path) -> list[str]:
    """Return human-readable structural errors for a Modelica package tree."""
    errors: list[str] = []
    for package_file in sorted(package_root.rglob("package.mo")):
        package_dir = package_file.parent
        order_file = package_dir / "package.order"
        relative = package_dir.relative_to(package_root.parent)
        if not order_file.is_file():
            errors.append(f"{relative}: missing package.order")
            continue

        ordered = _ordered_entries(package_dir)
        declared = _declared_entries(package_dir)
        if len(ordered) != len(set(ordered)):
            errors.append(f"{relative}: duplicate package.order entries")
        if set(ordered) != declared:
            missing = sorted(declared - set(ordered))
            extra = sorted(set(ordered) - declared)
            errors.append(f"{relative}: package.order mismatch missing={missing} extra={extra}")

        source = package_file.read_text(encoding="utf-8")
        if "Documentation(info=" not in source:
            errors.append(f"{relative}: package.mo lacks Documentation(info=...)")

    root_source = (package_root / "package.mo").read_text(encoding="utf-8")
    compact_source = "".join(root_source.split())
    expected_uses = 'uses(Modelica(version="4.0.0"))'
    if compact_source.count("uses(") != 1 or expected_uses not in compact_source:
        errors.append(f'{PACKAGE_NAME}: expected exactly uses(Modelica(version="4.0.0"))')
    if 'version="0.1.0"' not in compact_source:
        errors.append(f'{PACKAGE_NAME}: expected version="0.1.0"')
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "package_root",
        nargs="?",
        type=Path,
        default=Path(__file__).resolve().parents[1] / PACKAGE_NAME,
    )
    errors = validate_package(parser.parse_args().package_root.resolve())
    if errors:
        print("\n".join(errors))
        return 1
    print(f"{PACKAGE_NAME} package structure is valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
