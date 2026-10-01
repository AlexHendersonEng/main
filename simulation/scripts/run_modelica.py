import subprocess
from OMPython import OMCSessionLocal
from pathlib import Path
import sys
from simulation.results import load_mat
import matplotlib.pyplot as plt


def main():
    # Variables
    model_file = (Path(__file__).parent / "MassSpringDamper.mo").as_posix()
    model_name = "MassSpringDamper"
    t_start = 0.0
    t_stop = 10.0
    number_of_intervals = 1000
    tolerance = 1e-6
    method = "dassl"
    options = ""
    output_format = "mat"
    variable_filter = ".*"
    c_flags = ""
    sim_flags = ""

    # Create build directory if it doesn't exist
    build_dir = Path(__file__).parent / "build"
    build_dir.mkdir(parents=True, exist_ok=True)
    file_name_prefix = (build_dir / model_name).as_posix()

    # Create an OMC session
    omc = OMCSessionLocal()
    _ = omc.sendExpression('setCommandLineOptions("-d=initialization")')

    # Build the model
    _ = omc.sendExpression(f'loadFile("{model_file}")')
    _ = omc.sendExpression(
        f"buildModel("
        f"{model_name}, "
        f"startTime={t_start}, "
        f"stopTime={t_stop}, "
        f"numberOfIntervals={number_of_intervals}, "
        f"tolerance={tolerance}, "
        f'method="{method}", '
        f'fileNamePrefix="{file_name_prefix}", '
        f'options="{options}", '
        f'outputFormat="{output_format}", '
        f'variableFilter="{variable_filter}", '
        f'cflags="{c_flags}", '
        f'simflags="{sim_flags}"'
        f")"
    )

    # Run the simulation
    if sys.platform == "win32":
        executable = f"{file_name_prefix}.exe"
    else:
        executable = f"{file_name_prefix}"

    subprocess.run([executable], check=True)

    # Load the simulation results
    result_file = f"{file_name_prefix}_res.mat"
    results = load_mat(result_file)
    print(results.variable_names())

    # Plot the results
    plt.figure()
    plt.plot(results.data["time"], results.data["x"], label="Position")
    plt.plot(results.data["time"], results.data["v"], label="Velocity")
    plt.grid()
    plt.xlabel("Time")
    plt.ylabel("Values")
    plt.title("Simulation Results")
    plt.legend()
    plt.show()

    # Gracefully terminate the OMC session
    omc.sendExpression("quit()")


if __name__ == "__main__":
    main()
