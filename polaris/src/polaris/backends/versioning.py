"""Version parsing and per-version adapter selection.

Backends keep a table of ``VersionAdapter`` entries so behaviour that changes
between compiler releases (CLI flags, output formats) lives in one place.
"""

from __future__ import annotations

import re
from collections.abc import Sequence
from dataclasses import dataclass, field
from functools import total_ordering
from typing import Any

_VERSION_RE = re.compile(r"(\d+)(?:\.(\d+))?(?:\.(\d+))?")


@total_ordering
@dataclass(frozen=True, slots=True)
class Version:
    major: int
    minor: int = 0
    patch: int = 0

    @classmethod
    def parse(cls, text: str) -> Version:
        match = _VERSION_RE.search(text)
        if match is None:
            raise ValueError(f"No version found in {text!r}")
        return cls(*(int(g) if g else 0 for g in match.groups()))

    def _key(self) -> tuple[int, int, int]:
        return (self.major, self.minor, self.patch)

    def __lt__(self, other: object) -> bool:
        if not isinstance(other, Version):
            return NotImplemented
        return self._key() < other._key()

    def __str__(self) -> str:
        return f"{self.major}.{self.minor}.{self.patch}"


@dataclass(frozen=True, slots=True)
class VersionAdapter:
    """Settings that apply to compiler versions >= ``min_version``."""

    min_version: Version
    settings: dict[str, Any] = field(default_factory=dict)


def select_adapter(version: Version, adapters: Sequence[VersionAdapter]) -> VersionAdapter:
    """Pick the adapter with the highest ``min_version`` not above ``version``.

    Versions newer than every entry use the newest adapter, so compiler updates
    work by default until an incompatibility is recorded.
    """
    eligible = [a for a in adapters if a.min_version <= version]
    if not eligible:
        raise ValueError(f"Version {version} is older than every supported adapter")
    return max(eligible, key=lambda a: a.min_version)
