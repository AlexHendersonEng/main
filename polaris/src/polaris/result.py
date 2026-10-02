"""Simulation results: a time vector plus named variable trajectories."""

from __future__ import annotations

import csv
from collections.abc import Iterator, Mapping
from dataclasses import dataclass, field
from pathlib import Path

import numpy as np
import pandas as pd


@dataclass(frozen=True)
class Result(Mapping[str, np.ndarray]):
    """Trajectories of one simulation run.

    Behaves as a read-only mapping from variable name to a numpy array that is
    aligned with ``time``. ``metadata`` records how the result was produced
    (backend, model, options) so saved results stay self-describing.
    """

    time: np.ndarray
    variables: dict[str, np.ndarray]
    metadata: dict[str, str] = field(default_factory=dict)

    def __getitem__(self, name: str) -> np.ndarray:
        if name == "time":
            return self.time
        try:
            return self.variables[name]
        except KeyError:
            known = ", ".join(sorted(self.variables)[:10])
            raise KeyError(f"No variable '{name}' in result. Available: {known} ...") from None

    def __iter__(self) -> Iterator[str]:
        return iter(self.variables)

    def __len__(self) -> int:
        return len(self.variables)

    def to_dataframe(self) -> pd.DataFrame:
        """Variables as columns, indexed by time."""
        return pd.DataFrame(self.variables, index=pd.Index(self.time, name="time"))

    def final(self) -> dict[str, float]:
        """Value of every variable at the last time point."""
        return {name: float(values[-1]) for name, values in self.variables.items()}

    def save(self, path: str | Path) -> None:
        """Write a CSV with a ``time`` column; metadata goes into ``#`` comment lines."""
        with Path(path).open("w", newline="", encoding="utf-8") as fh:
            for key, value in self.metadata.items():
                fh.write(f"# {key}: {value}\n")
            self.to_dataframe().to_csv(fh)

    @classmethod
    def load(cls, path: str | Path) -> Result:
        """Read a result written by :meth:`save` or by a backend."""
        return cls.from_csv(path)

    @classmethod
    def from_csv(cls, path: str | Path, metadata: dict[str, str] | None = None) -> Result:
        """Parse a CSV whose first column is time.

        Accepts both backend output (OpenModelica quotes its headers) and files
        written by :meth:`save`.
        """
        meta = dict(metadata or {})
        with Path(path).open(newline="", encoding="utf-8") as fh:
            lines = fh.readlines()
        # Leading "# key: value" lines are metadata written by save().
        body = [ln for ln in lines if not ln.startswith("#")]
        for ln in lines[: len(lines) - len(body)]:
            key, _, value = ln[1:].partition(":")
            meta.setdefault(key.strip(), value.strip())
        rows = list(csv.reader(body))
        if len(rows) < 2:
            raise ValueError(f"{path} contains no data rows")
        header = [h.strip() for h in rows[0]]
        data = np.array(rows[1:], dtype=float)
        if header[0] != "time":
            raise ValueError(f"{path}: first column must be 'time', got {header[0]!r}")
        variables = {name: data[:, i] for i, name in enumerate(header) if i > 0}
        return cls(time=data[:, 0], variables=variables, metadata=meta)
