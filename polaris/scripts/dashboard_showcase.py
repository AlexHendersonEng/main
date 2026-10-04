"""Launch the mass-spring-damper dashboard.

From the polaris directory, install the optional dashboard dependency and run:

    uv run --extra dashboard python scripts/dashboard_showcase.py

Use ``--backend rumoca`` to select Rumoca, or ``--host 0.0.0.0`` to make the dashboard
available on the local network. By default the server only binds to this computer.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from polaris.backends import available_backends
from polaris.interactive import InteractiveSession, run_dashboard
from polaris.model import Model

HERE = Path(__file__).parent


def main() -> None:
    """Build a Co-Simulation FMU and serve an interactive dashboard for it."""
    backends = available_backends()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--backend",
        choices=["auto", *backends],
        default="auto",
        help="Modelica compiler used to export the FMU (default: first installed backend)",
    )
    parser.add_argument("--host", default="127.0.0.1", help="Dashboard bind address")
    parser.add_argument("--port", type=int, default=8050, help="Dashboard HTTP port")
    parser.add_argument("--debug", action="store_true", help="Enable Dash development debug mode")
    args = parser.parse_args()

    model = Model("MassSpringDamper", [HERE / "MassSpringDamper.mo"])
    backend = None if args.backend == "auto" else args.backend

    # OpenModelica marks its FMU parameters fixed, so force_fixed_parameters enables
    # sliders for k and c there; Rumoca can update its tunable parameters normally.
    with InteractiveSession.from_model(
        model,
        backend=backend,
        step_size=0.01,
        substeps=20,
    ) as session:
        print(f"Serving {model.class_name} dashboard at http://{args.host}:{args.port}")
        print("Adjust k or c, then select Step; Reset restarts from the slider values.")
        run_dashboard(
            session,
            {"k": (5.0, 50.0), "c": (0.0, 10.0)},
            variables=("x", "v"),
            step_duration=0.1,
            force_fixed_parameters=True,
            host=args.host,
            port=args.port,
            debug=args.debug,
        )


if __name__ == "__main__":
    main()
