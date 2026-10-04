"""Dash UI that controls an existing interactive FMU session."""

from __future__ import annotations

import threading
from collections.abc import Mapping, Sequence
from math import isfinite
from typing import TYPE_CHECKING, Any

from polaris.interactive.session import InteractiveSession

if TYPE_CHECKING:
    from dash import Dash


def create_dashboard(
    session: InteractiveSession,
    control_ranges: Mapping[str, tuple[float, float]],
    *,
    variables: Sequence[str] | None = None,
    step_duration: float | None = None,
    force_fixed_parameters: bool = False,
    title: str = "Polaris interactive simulation",
) -> Dash:
    """Create a Dash app with sliders, step/reset controls, and live trajectories.

    ``dash`` is an optional dependency; install Polaris with ``pip install polaris[dashboard]``.
    The app controls the supplied session in-process and is intended for one local user.
    The returned app does not close the session: the caller retains lifecycle ownership.

    Args:
        session: An already created FMU session.
        control_ranges: Slider bounds for FMU input or parameter variables. Explicit
            bounds avoid guessing meaningful parameter ranges or units.
        variables: Variables to plot; defaults to recorded variables.
        step_duration: Simulated seconds advanced per click. Defaults to one communication
            step. The actual duration is rounded up to a whole number of session steps.
        force_fixed_parameters: Allow slider writes to FMI-fixed parameters, which is
            non-standard and backend-dependent (useful for OpenModelica FMUs).
        title: Heading shown in the browser.

    Raises:
        ImportError: Dash is not installed.
        KeyError / ValueError: A control or plotted variable is invalid.
    """
    try:
        from dash import Dash, Input, Output, State, ctx, dcc, html
        from plotly import graph_objects as go
    except ImportError as exc:
        raise ImportError(
            "The dashboard requires the optional 'dashboard' extra; "
            "install it with `pip install polaris[dashboard]`."
        ) from exc

    if step_duration is not None and step_duration <= 0:
        raise ValueError("step_duration must be positive")

    for name, bounds in control_ranges.items():
        if name not in session.variables:
            raise KeyError(f"Unknown FMU control variable '{name}'")
        if (
            len(bounds) != 2
            or not all(isfinite(value) for value in bounds)
            or bounds[0] >= bounds[1]
        ):
            raise ValueError(f"Slider bounds for '{name}' must be (minimum, maximum)")
        variable = session._variables[name]
        if variable.type != "Real":
            raise ValueError(
                f"Slider controls currently require a Real variable; '{name}' is {variable.type}"
            )
        if variable.causality not in ("input", "parameter"):
            raise ValueError(f"'{name}' is neither an FMU input nor a parameter")
        if (
            variable.causality == "parameter"
            and variable.variability != "tunable"
            and not force_fixed_parameters
        ):
            raise ValueError(
                f"'{name}' is fixed by the FMU; set force_fixed_parameters=True to expose it"
            )

    plotted = list(variables) if variables is not None else list(session._recorded)
    if not plotted:
        raise ValueError("At least one variable must be plotted")
    for name in plotted:
        if name not in session._recorded:
            raise KeyError(f"'{name}' is not recorded by the session")

    app = Dash(__name__)
    controls = []
    for name, (minimum, maximum) in control_ranges.items():
        variable = session._variables[name]
        if session._started:
            initial = session.get(name)[name]
        else:
            # Reading would initialise the FMU and defeat configuring its start values.
            initial = float(variable.start) if variable.start is not None else 0.0
        initial = min(max(initial, minimum), maximum)
        controls.append(
            html.Div(
                [
                    html.Label(name, htmlFor=f"control-{name}"),
                    dcc.Slider(
                        id=f"control-{name}",
                        min=minimum,
                        max=maximum,
                        step=(maximum - minimum) / 100,
                        value=initial,
                        tooltip={"placement": "bottom", "always_visible": True},
                    ),
                ],
                className="polaris-control",
            )
        )

    graph = dcc.Graph(
        id="polaris-trajectory",
        figure=_figure(session, plotted, go),
        config={"displaylogo": False},
    )
    app.layout = html.Main(
        [
            html.H1(title),
            html.Div(controls, id="polaris-controls"),
            html.Div(
                [
                    html.Label("Advance (simulated seconds)", htmlFor="polaris-duration"),
                    dcc.Input(
                        id="polaris-duration",
                        type="number",
                        min=session.step_size,
                        step=session.step_size,
                        value=step_duration or session.step_size,
                    ),
                    html.Button("Step", id="polaris-step", n_clicks=0),
                    html.Button("Reset", id="polaris-reset", n_clicks=0),
                    html.Span(id="polaris-status", role="status"),
                ],
                className="polaris-actions",
            ),
            graph,
        ],
        style={"maxWidth": "1100px", "margin": "2rem auto", "padding": "0 1rem"},
    )

    # FMI instances are not generally thread-safe; serialize Dash callbacks around the one
    # session supplied to this app, even if the server handles requests concurrently.
    session_lock = threading.RLock()

    @app.callback(
        Output("polaris-trajectory", "figure"),
        Output("polaris-status", "children"),
        Input("polaris-step", "n_clicks"),
        Input("polaris-reset", "n_clicks"),
        State("polaris-duration", "value"),
        *[State(f"control-{name}", "value") for name in control_ranges],
        prevent_initial_call=True,
    )
    def update(
        _step_clicks: int,
        _reset_clicks: int,
        duration: float | None,
        *slider_values: float,
    ) -> tuple[Any, str]:
        """Apply controls and redraw after a step or reset."""
        if ctx.triggered_id not in ("polaris-step", "polaris-reset"):
            return _figure(session, plotted, go), "No action"
        values = dict(zip(control_ranges, slider_values, strict=True))
        with session_lock:
            try:
                if ctx.triggered_id == "polaris-reset":
                    parameters = {
                        name: value
                        for name, value in values.items()
                        if session._variables[name].causality == "parameter"
                    }
                    session.reset(parameters=parameters)
                    session.set(
                        {
                            name: value
                            for name, value in values.items()
                            if session._variables[name].causality == "input"
                        }
                    )
                    message = f"Reset to t={session.time:g}"
                else:
                    session.set(values, force=force_fixed_parameters)
                    actual_duration = duration or session.step_size
                    session.advance(actual_duration)
                    message = f"t={session.time:g}"
                return _figure(session, plotted, go), message
            except Exception as exc:
                return _figure(session, plotted, go), f"Error: {exc}"

    return app


def run_dashboard(
    session: InteractiveSession,
    control_ranges: Mapping[str, tuple[float, float]],
    *,
    host: str = "127.0.0.1",
    port: int = 8050,
    debug: bool = False,
    **options: Any,
) -> None:
    """Create and run a dashboard server.

    Args:
        session: Session controlled by the dashboard; caller must close it after the server exits.
        control_ranges: Explicit slider bounds for exposed FMU inputs and parameters.
        host: Bind address; defaults to loopback so the dashboard is not public by accident.
        port: Local HTTP port.
        debug: Enable Dash's development server debugger.
        **options: Other :func:`create_dashboard` options.
    """
    app = create_dashboard(session, control_ranges, **options)
    app.run(host=host, port=port, debug=debug)


def _figure(session: InteractiveSession, variables: Sequence[str], go: Any) -> Any:
    """Build a Plotly figure using the session's current in-memory history."""
    history = session.history
    figure = go.Figure()
    for name in variables:
        figure.add_trace(go.Scatter(x=history.time, y=history[name], mode="lines", name=name))
    figure.update_layout(
        xaxis_title="Time",
        yaxis_title="Value",
        legend={"orientation": "h"},
        margin={"l": 40, "r": 20, "t": 30, "b": 40},
    )
    return figure
