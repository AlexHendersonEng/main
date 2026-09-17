import casadi as ca
import numpy as np
import matplotlib.pyplot as plt


def plot_sensitivities(t_grid, dpos_dk, dpos_dc, dvel_dk, dvel_dc, title_prefix):
    fig, axes = plt.subplots(2, 1, figsize=(8, 6), sharex=True)

    axes[0].plot(t_grid, dpos_dk, label="d position / dk")
    axes[0].plot(t_grid, dpos_dc, label="d position / dc")
    axes[0].set_ylabel("Position sensitivity")
    axes[0].set_title(f"{title_prefix} sensitivities")
    axes[0].legend()
    axes[0].grid()

    axes[1].plot(t_grid, dvel_dk, label="d velocity / dk")
    axes[1].plot(t_grid, dvel_dc, label="d velocity / dc")
    axes[1].set_xlabel("t (s)")
    axes[1].set_ylabel("Velocity sensitivity")
    axes[1].legend()
    axes[1].grid()

    plt.tight_layout()
    plt.show()


def main():
    mass = 1.0
    x0 = ca.DM([1.0, 0.0])
    p0 = ca.DM([2.0, 0.2])  # [k, c]
    t_grid = np.linspace(0.0, 10.0, 100)
    n_times = len(t_grid)

    # ODE state x = [position, velocity], parameters p = [k, c]
    x = ca.MX.sym("x", 2)
    p = ca.MX.sym("p", 2)
    xdot = ca.vertcat(x[1], -(p[0] * x[0] + p[1] * x[1]) / mass)
    dae = {"x": x, "p": p, "ode": xdot}

    integrator = ca.integrator("sim", "cvodes", dae, 0.0, t_grid)
    sim_res = integrator(x0=x0, p=p0)
    xf = sim_res["xf"]

    # Plot nominal trajectory.
    plt.figure(figsize=(8, 4))
    plt.plot(t_grid, xf[0, :].full().ravel(), label="position")
    plt.plot(t_grid, xf[1, :].full().ravel(), label="velocity")
    plt.xlabel("t (s)")
    plt.ylabel("State")
    plt.title("Mass-spring-damper response")
    plt.legend()
    plt.grid()
    plt.tight_layout()
    plt.show()

    # Forward sensitivities
    integrator_fwd = integrator.factory("F_fwd", ["x0", "p", "fwd:p"], ["fwd:xf"])
    res_fwd = integrator_fwd(x0=x0, p=p0, fwd_p=ca.DM.eye(2))
    xf_fwd = res_fwd["fwd_xf"]

    dpos_dk_fwd = xf_fwd[0, :n_times].full().ravel()
    dpos_dc_fwd = xf_fwd[0, n_times:].full().ravel()
    dvel_dk_fwd = xf_fwd[1, :n_times].full().ravel()
    dvel_dc_fwd = xf_fwd[1, n_times:].full().ravel()

    plot_sensitivities(
        t_grid,
        dpos_dk_fwd,
        dpos_dc_fwd,
        dvel_dk_fwd,
        dvel_dc_fwd,
        "Forward",
    )

    # Adjoint sensitivities
    integrator_adj = integrator.factory("F_adj", ["x0", "p", "adj:xf"], ["adj:p"])

    dpos_dk_adj = np.zeros(n_times)
    dpos_dc_adj = np.zeros(n_times)
    dvel_dk_adj = np.zeros(n_times)
    dvel_dc_adj = np.zeros(n_times)

    # Reconstruct trajectory sensitivities by seeding one output at a time.
    for i in range(n_times):
        seed_pos = ca.DM.zeros(2, n_times)
        seed_pos[0, i] = 1.0
        grad_pos = integrator_adj(x0=x0, p=p0, adj_xf=seed_pos)["adj_p"]
        dpos_dk_adj[i] = float(grad_pos[0])
        dpos_dc_adj[i] = float(grad_pos[1])

        seed_vel = ca.DM.zeros(2, n_times)
        seed_vel[1, i] = 1.0
        grad_vel = integrator_adj(x0=x0, p=p0, adj_xf=seed_vel)["adj_p"]
        dvel_dk_adj[i] = float(grad_vel[0])
        dvel_dc_adj[i] = float(grad_vel[1])

    plot_sensitivities(
        t_grid,
        dpos_dk_adj,
        dpos_dc_adj,
        dvel_dk_adj,
        dvel_dc_adj,
        "Adjoint",
    )


if __name__ == "__main__":
    main()
