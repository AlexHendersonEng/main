# Polaris

## Interactive dashboard

Install the optional Dash dependency with `uv sync --extra dashboard`. An
`InteractiveSession` can then be exposed through a local browser dashboard:

```python
from pathlib import Path

from polaris import Model
from polaris.interactive import InteractiveSession, run_dashboard

model = Model("MassSpringDamper", [Path("scripts/MassSpringDamper.mo")])
with InteractiveSession.from_model(model, backend="openmodelica", step_size=0.01) as session:
    run_dashboard(
        session,
        {"k": (5.0, 50.0), "c": (0.0, 10.0)},
        variables=("x", "v"),
        step_duration=0.1,
        # OpenModelica marks parameters fixed in its FMUs; this opts into its live-write behaviour.
        force_fixed_parameters=True,
    )
```

Slider bounds are explicit because useful ranges depend on the model and units.
The dashboard is local-only by default (`127.0.0.1`), controls one session at a time,
and updates the trajectory after each Step or Reset action. Inputs and tunable parameters
can be changed normally; fixed parameters require the explicit `force_fixed_parameters`
opt-in and may not be live in every FMU.
