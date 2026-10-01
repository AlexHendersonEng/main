from OMPython import OMCSessionLocal
from pathlib import Path
import os
from fmpy import simulate_fmu
from fmpy.util import plot_result


def main():
    # Variables
    model_file = (Path(__file__).parent / "MassSpringDamper.mo").as_posix()
    model_name = "MassSpringDamper"
    fmu_version = "2.0"
    fmu_type = "cs"
    platforms = "static"
    include_resources = "false"

    # Create build directory if it doesn't exist
    build_dir = Path(__file__).parent / "build"
    build_dir.mkdir(parents=True, exist_ok=True)

    # Create an OMC session
    os.chdir(build_dir)
    omc = OMCSessionLocal()

    # Translate the model
    curr_dir = Path().cwd()
    _ = omc.sendExpression(f'loadFile("{model_file}")')
    _ = omc.sendExpression(
        f"buildModelFMU("
        f"{model_name}, "
        f'version="{fmu_version}", '
        f'fmuType="{fmu_type}", '
        f'fileNamePrefix="{(build_dir / model_name).relative_to(curr_dir).as_posix()}", '
        rf'platforms={{"{platforms}"}}, '
        f"includeResources={include_resources}"
        f")"
    )

    # Gracefully terminate the OMC session
    omc.sendExpression("quit()")

    # Run the simulation
    result = simulate_fmu(
        f"{build_dir / model_name}.fmu", start_time=0.0, stop_time=10.0
    )

    # Plot the result
    plot_result(result)


if __name__ == "__main__":
    main()
