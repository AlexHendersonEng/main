"""Matplotlib plotting helpers for simulation results.

All functions return the matplotlib ``Figure`` so callers can tweak or save it; none of
them call ``plt.show()``. Figures are built with the object-oriented ``Figure`` API (not
pyplot), so they never touch global GUI state and work headless; use ``fig.show()`` or
``save_to`` to view them.
"""

from __future__ import annotations

from collections.abc import Mapping, Sequence
from pathlib import Path

from matplotlib.axes import Axes
from matplotlib.figure import Figure

from polaris.result import Result


def _names(result: Result, variables: Sequence[str] | None) -> list[str]:
    """Requested variables, validated; defaults to every variable."""
    names = list(variables) if variables else list(result)
    for name in names:
        result[name]  # raises a helpful KeyError for unknown names
    return names


def plot(
    result: Result,
    variables: Sequence[str] | None = None,
    *,
    separate: bool = False,
    title: str | None = None,
    save_to: str | Path | None = None,
) -> Figure:
    """Plot variables against time.

    Args:
        result: The simulation result.
        variables: Names to plot; all variables when omitted.
        separate: One stacked subplot per variable (shared time axis) instead of one
            axes with every line. Useful when variables have very different scales.
        title: Optional figure title.
        save_to: If given, the figure is also written to this path.
    """
    names = _names(result, variables)
    if not names:
        raise ValueError("Nothing to plot: the result has no variables")

    fig = Figure(figsize=(8, 2.5 * len(names) if separate else 5), layout="constrained")
    if separate:
        axes = fig.subplots(len(names), 1, sharex=True, squeeze=False)[:, 0]
        for ax, name in zip(axes, names, strict=True):
            ax.plot(result.time, result[name])
            ax.set_ylabel(name)
            ax.grid(True)
        axes[-1].set_xlabel("time")
    else:
        ax = fig.subplots()
        for name in names:
            ax.plot(result.time, result[name], label=name)
        ax.set_xlabel("time")
        ax.grid(True)
        ax.legend()
    if title:
        fig.suptitle(title)
    if save_to is not None:
        fig.savefig(save_to)
    return fig


def compare(
    results: Mapping[str, Result],
    variable: str,
    *,
    title: str | None = None,
    save_to: str | Path | None = None,
) -> Figure:
    """Overlay one variable from several results, e.g. different backends or parameter values.

    Args:
        results: Label -> result. Each result is drawn on its own time base, so results
            with different output grids can be compared directly.
        variable: Variable present in every result.
    """
    if not results:
        raise ValueError("compare() needs at least one result")
    fig = Figure(figsize=(8, 5), layout="constrained")
    ax: Axes = fig.subplots()
    for label, result in results.items():
        ax.plot(result.time, result[variable], label=label)
    ax.set_xlabel("time")
    ax.set_ylabel(variable)
    ax.grid(True)
    ax.legend()
    if title:
        fig.suptitle(title)
    if save_to is not None:
        fig.savefig(save_to)
    return fig
