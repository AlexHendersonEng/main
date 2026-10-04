"""Live plotting of an :class:`~polaris.interactive.session.InteractiveSession`."""

from __future__ import annotations

from collections.abc import Sequence

from matplotlib.figure import Figure

from polaris.interactive.session import InteractiveSession


class LivePlot:
    """A figure whose lines are refreshed from a session's history.

    The class only manages the figure; displaying it is up to the caller. That keeps it
    usable headless and inside any GUI or notebook backend (call :meth:`update` whenever
    new steps have been taken). :func:`run_live` is a ready-made loop for plain scripts.

    Args:
        variables: Names to plot, one subplot each (shared time axis).
        figure: Figure to draw into, e.g. one made by pyplot so it can be shown. A new
            object-oriented ``Figure`` is created when omitted.
        window: If given, only the most recent ``window`` seconds are shown.
    """

    def __init__(
        self,
        variables: Sequence[str],
        *,
        figure: Figure | None = None,
        window: float | None = None,
    ) -> None:
        if not variables:
            raise ValueError("LivePlot needs at least one variable")
        self.variables = list(variables)
        self.window = window
        self.figure = figure or Figure(figsize=(8, 2.5 * len(self.variables)))
        axes = self.figure.subplots(len(self.variables), 1, sharex=True, squeeze=False)[:, 0]
        self._axes = list(axes)
        self._lines = []
        for ax, name in zip(self._axes, self.variables, strict=True):
            (line,) = ax.plot([], [])
            ax.set_ylabel(name)
            ax.grid(True)
            self._lines.append(line)
        self._axes[-1].set_xlabel("time")

    def update(self, session: InteractiveSession) -> Figure:
        """Redraw from the session's history and return the figure."""
        history = session.history
        time = history.time
        first = 0
        if self.window is not None and len(time):
            # time is sorted, so a binary search finds where the visible window begins.
            first = int(time.searchsorted(time[-1] - self.window))
        for ax, line, name in zip(self._axes, self._lines, self.variables, strict=True):
            line.set_data(time[first:], history[name][first:])
            ax.relim()
            ax.autoscale_view()
        return self.figure


def run_live(
    session: InteractiveSession,
    variables: Sequence[str],
    duration: float,
    *,
    refresh_every: int = 5,
    window: float | None = None,
    pause: float = 0.001,
) -> Figure:
    """Advance ``session`` by ``duration`` while showing a window that updates as it runs.

    Args:
        session: The session to run; its existing history stays on the plot.
        variables: Variables to plot.
        duration: Simulated time to run for.
        refresh_every: Redraw after this many steps. Drawing is much slower than stepping,
            so raise it for fast models.
        window: Show only the most recent ``window`` seconds.
        pause: GUI event-loop time per refresh, in seconds.
    """
    # pyplot is imported here, not at module level, because it is what creates GUI windows
    # and the rest of polaris deliberately avoids global GUI state.
    import matplotlib.pyplot as plt

    if refresh_every < 1:
        raise ValueError("refresh_every must be at least 1")
    live = LivePlot(variables, figure=plt.figure(), window=window)
    remaining = int(duration / session.step_size + 0.5)
    while remaining > 0:
        chunk = min(refresh_every, remaining)
        session.step(chunk)
        remaining -= chunk
        live.update(session)
        # matplotlib treats a pause of 0 as "wait forever", so enforce a tiny minimum.
        plt.pause(max(pause, 1e-3))
    return live.figure
