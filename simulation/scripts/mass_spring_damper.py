import casadi as ca
import numpy as np
import matplotlib.pyplot as plt


def main():
    # Mass spring damper parameters
    mass = 1.0  # mass
    k = 2.0  # spring constant
    c = 0.2  # damping coefficient

    # Differential Algebraic Equation (DAE) model for a mass spring damper system
    dae = ca.DaeBuilder("mass_spring_damper")

    # Model variables
    _ = dae.add("t", "independent")
    position = dae.add("position", "output", dict(start=1.0, initial="exact"))
    velocity = dae.add("velocity", "output", dict(start=0.0, initial="exact"))

    # Dynamic equations
    dae.eq(dae.der(position), velocity)
    dae.eq(dae.der(velocity), -(k * position + c * velocity) / mass)
    dae.disp(True)

    # Default experiment
    dae.set_start_time(0.0)
    dae.set_stop_time(10.0)

    # Simulate
    tgrid = np.linspace(dae.start_time(), dae.stop_time(), 100)
    sim = ca.integrator(
        "sim", "cvodes", dae.create(), 0, tgrid, dict(transition=dae.transition())
    )
    simres = sim(x0=dae.start(dae.x()))

    # Visualize the solution
    plt.figure(1)
    plt.plot(tgrid, simres["xf"][0, :].T)
    plt.xlabel("t (s)")
    plt.ylabel("position (m)")
    plt.grid()
    plt.show()


if __name__ == "__main__":
    main()
