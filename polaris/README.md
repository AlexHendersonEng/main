# Polaris

Polaris is a Python library and CLI for running Modelica models through multiple
compiler backends, exporting and running FMUs, and analysing model behaviour.

## Installation

Polaris requires Python 3.14 or newer. With `uv`, install the project and its
development tools with:

```powershell
uv sync
```

Install optional browser-dashboard support when needed:

```powershell
uv sync --extra dashboard
```

Install OpenModelica (`omc`) and/or Rumoca separately to use those compiler
backends. The CLI's `backends` command reports which registered compilers are
available on the current machine.

## Command-line use

Commands can be run from an installed environment with `polaris ...`, or during
development with `uv run polaris ...`. Model commands take a fully qualified
class name and one or more Modelica source files.

```powershell
# Simulate, save a CSV, and choose recorded variables.
uv run polaris simulate MassSpringDamper --file scripts\MassSpringDamper.mo `
  --backend rumoca --stop 5 --step 0.01 --variable x --variable v `
  --output scripts\output\mass_spring.csv

# Export and inspect an FMU.
uv run polaris fmu export MassSpringDamper scripts\output\MassSpringDamper.fmu `
  --file scripts\MassSpringDamper.mo --backend rumoca
uv run polaris fmu inspect scripts\output\MassSpringDamper.fmu
uv run polaris fmu validate scripts\output\MassSpringDamper.fmu

# Run an FMU and save the result.
uv run polaris fmu run scripts\output\MassSpringDamper.fmu --stop 5 `
  --step 0.01 --output scripts\output\mass_spring_fmu.csv

# Compute a native Jacobian (Rumoca currently provides this capability).
uv run polaris jacobian MassSpringDamper --file scripts\MassSpringDamper.mo `
  --backend rumoca --at x=1 --at v=0

# Local and global sensitivity analysis.
uv run polaris sensitivity local MassSpringDamper --file scripts\MassSpringDamper.mo `
  --backend rumoca --parameter k=20 --parameter c=0.5 --variable x --stop 5
uv run polaris sensitivity global MassSpringDamper --file scripts\MassSpringDamper.mo `
  --backend rumoca --bound k=10,40 --bound c=0.1,2 --output-variable x `
  --method sobol --samples 32 --stop 5

# Serve an FMU through the optional interactive dashboard.
uv run polaris dashboard scripts\output\MassSpringDamper.fmu `
  --control k=5,50 --control c=0,10 --variable x --variable v `
  --force-fixed-parameters

# See compiler versions and capabilities.
uv run polaris backends
```

Simulation commands can repeat `--parameter NAME=VALUE`; global sensitivity
commands use `--bound NAME=MIN,MAX`. The dashboard requires explicit slider
bounds, binds to `127.0.0.1` by default, and requires `--force-fixed-parameters`
to expose FMU parameters declared fixed (as OpenModelica commonly exports them).
Use `uv run polaris COMMAND --help` for full command-specific options.

## HELICS FMU federates

The optional HELICS integration runs one FMI 2.0 Co-Simulation FMU as a value
federate. Install its Python bindings with `uv sync --extra helics`. The
`FmuFederate` API takes explicit FMU-variable-to-HELICS-key mappings and
synchronizes the FMU communication step with HELICS time grants:

```python
from polaris.cosim import FmuFederate

producer = FmuFederate(
    "producer.fmu",
    "producer",
    outputs={"y": "producer.y"},
    step_size=0.05,
    broker="local-broker",
)
```

The broker and other federates are application-owned; a small two-FMU example is
provided in `scripts/helics_two_fmu.py`:

```powershell
uv run --extra helics python scripts\helics_two_fmu.py
```

It connects the producer's `y` publication to the consumer's `u` input and writes
both results, plus producer/consumer/coupling plots, under `scripts/output/`. Both
federates must run concurrently because HELICS time grants synchronize their
execution. Source-only Rumoca FMUs are compiled locally and therefore require a C
compiler.

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
