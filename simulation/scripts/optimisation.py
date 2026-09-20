from __future__ import annotations
import json
from pathlib import Path
import casadi as ca
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


MASS = 1.0
TRUE_PARAMS = np.array([2.0, 0.2], dtype=float)  # [spring stiffness, damping]
INITIAL_STATE = np.array([1.0, 0.0], dtype=float)  # [position, velocity]
INITIAL_GUESS = np.array([0.8, 0.6], dtype=float)
T_FINAL = 10.0
N_STEPS = 200


class IterationHistoryCallback(ca.Callback):
    """Collect IPOPT iteration data so the optimisation history can be plotted."""

    def __init__(
        self,
        name: str,
        x_shape: tuple[int, int],
        g_shape: tuple[int, int],
        p_shape: tuple[int, int],
    ):
        self.x_shape = x_shape
        self.g_shape = g_shape
        self.p_shape = p_shape
        self.iteration_x: list[np.ndarray] = []
        self.iteration_f: list[float] = []
        super().__init__()
        self.construct(name)

    def get_n_in(self) -> int:
        return 6

    def get_n_out(self) -> int:
        return 1

    def get_name_in(self, i: int) -> str:
        return ca.nlpsol_out()[i]

    def get_name_out(self, i: int) -> str:
        return ["ret"][i]

    def get_sparsity_in(self, i: int) -> ca.Sparsity:
        if i == 0:
            return ca.Sparsity.dense(*self.x_shape)
        if i == 1:
            return ca.Sparsity.dense(1, 1)
        if i == 2:
            return ca.Sparsity.dense(*self.g_shape)
        if i == 3:
            return ca.Sparsity.dense(*self.x_shape)
        if i == 4:
            return ca.Sparsity.dense(*self.g_shape)
        if i == 5:
            return ca.Sparsity.dense(*self.p_shape)
        raise IndexError(i)

    def get_sparsity_out(self, i: int) -> ca.Sparsity:
        if i == 0:
            return ca.Sparsity.dense(1, 1)
        raise IndexError(i)

    def eval(self, arg):
        x = np.asarray(arg[0].full()).reshape(-1)
        f = float(arg[1])
        self.iteration_x.append(x)
        self.iteration_f.append(f)
        return [0]


def build_step_integrator(dt: float) -> ca.Function:
    """Build a one-step CasADi integrator for the mass-spring-damper ODE."""
    x = ca.MX.sym("x", 2)
    p = ca.MX.sym("p", 2)  # p = [k, c]

    xdot = ca.vertcat(
        x[1],
        -(p[0] * x[0] + p[1] * x[1]) / MASS,
    )
    dae = {"x": x, "p": p, "ode": xdot}

    return ca.integrator(
        "step",
        "cvodes",
        dae,
        0.0,
        dt,
        {
            "abstol": 1e-10,
            "reltol": 1e-10,
        },
    )


def simulate_trajectory(
    step: ca.Function, x0: np.ndarray, params: np.ndarray, n_steps: int
) -> np.ndarray:
    """Simulate the trajectory over the full time horizon."""
    states = np.zeros((n_steps + 1, 2), dtype=float)
    states[0] = np.asarray(x0, dtype=float)

    x = ca.DM(x0)
    p = ca.DM(params)
    for i in range(n_steps):
        x = step(x0=x, p=p)["xf"]
        states[i + 1] = np.asarray(x.full()).reshape(-1)

    return states


def build_symbolic_loss(
    step: ca.Function, measured_states: np.ndarray
) -> tuple[ca.MX, ca.MX]:
    """Build a symbolic least-squares loss against measured states."""
    theta = ca.MX.sym("theta", 2)
    x = ca.MX(INITIAL_STATE)
    measured_dm = ca.DM(measured_states.T)  # shape: 2 x (N+1)

    loss = ca.sumsqr(x - measured_dm[:, 0])
    for i in range(1, measured_dm.size2()):
        x = step(x0=x, p=theta)["xf"]
        loss += ca.sumsqr(x - measured_dm[:, i])

    return theta, loss


def build_step_adjoint(step: ca.Function) -> ca.Function:
    """Build an adjoint function for one integration step."""
    return step.factory("step_adj", ["x0", "p", "adj:xf"], ["adj:x0", "adj:p"])


def adjoint_loss_and_gradient(
    step_adj: ca.Function,
    simulated_states: np.ndarray,
    measured_states: np.ndarray,
    params: np.ndarray,
) -> tuple[float, np.ndarray]:
    """Compute the trajectory loss and its gradient using adjoint propagation."""
    residuals = simulated_states - measured_states
    loss = float(np.sum(residuals**2))

    grad_p = np.zeros(2, dtype=float)
    adj_state_next = np.zeros(2, dtype=float)
    p_dm = ca.DM(params)

    for i in range(simulated_states.shape[0] - 1, 0, -1):
        adj_state = 2.0 * residuals[i] + adj_state_next
        step_res = step_adj(
            x0=ca.DM(simulated_states[i - 1]),
            p=p_dm,
            adj_xf=ca.DM(adj_state),
        )
        grad_p += np.asarray(step_res["adj_p"].full()).reshape(-1)
        adj_state_next = np.asarray(step_res["adj_x0"].full()).reshape(-1)

    return loss, grad_p


def optimise_parameters(
    step: ca.Function,
    measured_states: np.ndarray,
    initial_guess: np.ndarray,
) -> tuple[np.ndarray, list[np.ndarray]]:
    """Estimate the unknown spring stiffness and damping coefficient using CasADi's built-in NLP solver."""
    theta, loss = build_symbolic_loss(step, measured_states)
    nlp = {"x": theta, "f": loss}
    history_cb = IterationHistoryCallback(
        "history_cb",
        x_shape=(2, 1),
        g_shape=(0, 1),
        p_shape=(0, 1),
    )
    solver = ca.nlpsol(
        "solver",
        "ipopt",
        nlp,
        {
            "print_time": False,
            "iteration_callback": history_cb,
            "ipopt": {
                "print_level": 0,
                "max_iter": 100,
                "tol": 1e-10,
                "acceptable_tol": 1e-8,
                "mu_strategy": "adaptive",
            },
        },
    )

    sol = solver(
        x0=ca.DM(initial_guess),
        lbx=ca.DM([1e-8, 1e-8]),
        ubx=ca.DM([100.0, 100.0]),
    )
    estimated = np.asarray(sol["x"].full()).reshape(-1)
    return estimated, history_cb.iteration_x


def build_history_points(
    initial_guess: np.ndarray,
    iterates: list[np.ndarray],
    final_params: np.ndarray,
) -> list[np.ndarray]:
    """Assemble a clean parameter path for plotting and saved outputs."""
    path = [np.asarray(initial_guess, dtype=float)]
    for params in iterates:
        params = np.asarray(params, dtype=float)
        if not np.allclose(params, path[-1]):
            path.append(params)
    final_params = np.asarray(final_params, dtype=float)
    if not np.allclose(final_params, path[-1]):
        path.append(final_params)
    return path


def evaluate_history(
    grad_fun: ca.Function,
    parameter_path: list[np.ndarray],
) -> tuple[list[float], list[float]]:
    """Compute exact objective and gradient norms for each point in the parameter path."""
    loss_history: list[float] = []
    grad_norm_history: list[float] = []

    for params in parameter_path:
        loss_value, grad_value = grad_fun(ca.DM(params))
        loss_history.append(float(loss_value))
        grad_norm_history.append(
            float(np.linalg.norm(np.asarray(grad_value.full()).reshape(-1)))
        )

    return loss_history, grad_norm_history


def save_outputs(
    output_dir: Path,
    t_grid: np.ndarray,
    true_states: np.ndarray,
    initial_states: np.ndarray,
    estimated_states: np.ndarray,
    loss_history: list[float],
    grad_norm_history: list[float],
    true_params: np.ndarray,
    initial_guess: np.ndarray,
    estimated_params: np.ndarray,
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / ".gitignore").write_text("*")

    np.savez_compressed(
        output_dir / "mass_spring_damper_results.npz",
        t=t_grid,
        true_states=true_states,
        initial_guess_states=initial_states,
        estimated_states=estimated_states,
        true_params=true_params,
        initial_guess=initial_guess,
        estimated_params=estimated_params,
        loss_history=np.asarray(loss_history, dtype=float),
        grad_norm_history=np.asarray(grad_norm_history, dtype=float),
    )

    summary = {
        "mass": MASS,
        "true_params": true_params.tolist(),
        "initial_guess": initial_guess.tolist(),
        "estimated_params": estimated_params.tolist(),
        "absolute_error": np.abs(estimated_params - true_params).tolist(),
        "relative_error": (
            np.abs(estimated_params - true_params) / true_params
        ).tolist(),
        "final_loss": float(loss_history[-1]),
        "final_grad_norm": float(grad_norm_history[-1]),
    }
    with (output_dir / "summary.json").open("w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)


def save_plots(
    output_dir: Path,
    t_grid: np.ndarray,
    true_states: np.ndarray,
    initial_states: np.ndarray,
    estimated_states: np.ndarray,
    loss_history: list[float],
    grad_norm_history: list[float],
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / ".gitignore").write_text("*")

    fig, axes = plt.subplots(2, 1, figsize=(9, 7), sharex=True)
    axes[0].plot(t_grid, true_states[:, 0], label="true position", linewidth=2)
    axes[0].plot(t_grid, initial_states[:, 0], "--", label="initial guess position")
    axes[0].plot(
        t_grid, estimated_states[:, 0], ":", label="estimated position", linewidth=2
    )
    axes[0].set_ylabel("Position")
    axes[0].grid(True)
    axes[0].legend()

    axes[1].plot(t_grid, true_states[:, 1], label="true velocity", linewidth=2)
    axes[1].plot(t_grid, initial_states[:, 1], "--", label="initial guess velocity")
    axes[1].plot(
        t_grid, estimated_states[:, 1], ":", label="estimated velocity", linewidth=2
    )
    axes[1].set_xlabel("Time [s]")
    axes[1].set_ylabel("Velocity")
    axes[1].grid(True)
    axes[1].legend()

    fig.suptitle("Mass-spring-damper trajectory fitting")
    fig.tight_layout()
    fig.savefig(output_dir / "trajectory_fit.png", dpi=200)
    plt.close(fig)

    fig, axes = plt.subplots(2, 1, figsize=(9, 7), sharex=True)
    axes[0].semilogy(loss_history, marker="o")
    axes[0].set_ylabel("Loss")
    axes[0].grid(True, which="both")

    axes[1].semilogy(grad_norm_history, marker="o")
    axes[1].set_xlabel("Iteration")
    axes[1].set_ylabel("Gradient norm")
    axes[1].grid(True, which="both")

    fig.suptitle("Optimisation history")
    fig.tight_layout()
    fig.savefig(output_dir / "optimisation_history.png", dpi=200)
    plt.close(fig)


def main() -> None:
    t_grid = np.linspace(0.0, T_FINAL, N_STEPS + 1)
    dt = float(t_grid[1] - t_grid[0])
    results_dir = Path(__file__).resolve().parent / "results"

    step = build_step_integrator(dt)
    step_adj = build_step_adjoint(step)

    # Generate the synthetic "known" trajectory.
    true_states = simulate_trajectory(step, INITIAL_STATE, TRUE_PARAMS, N_STEPS)

    # Run a second simulation with incorrect parameters.
    initial_guess_states = simulate_trajectory(
        step, INITIAL_STATE, INITIAL_GUESS, N_STEPS
    )

    theta, loss = build_symbolic_loss(step, true_states)
    grad_fun = ca.Function("grad_fun", [theta], [loss, ca.gradient(loss, theta)])

    # Compare symbolic CasADi gradient against explicit adjoint propagation.
    initial_loss_symbolic, initial_grad_symbolic = grad_fun(ca.DM(INITIAL_GUESS))
    initial_loss_symbolic = float(initial_loss_symbolic)
    initial_grad_symbolic = np.asarray(initial_grad_symbolic.full()).reshape(-1)

    initial_loss_adjoint, initial_grad_adjoint = adjoint_loss_and_gradient(
        step_adj,
        initial_guess_states,
        true_states,
        INITIAL_GUESS,
    )

    np.testing.assert_allclose(
        initial_grad_adjoint,
        initial_grad_symbolic,
        rtol=1e-7,
        atol=1e-9,
    )

    # Solve the inverse problem with CasADi's built-in NLP solver.
    estimated_params, iterates = optimise_parameters(step, true_states, INITIAL_GUESS)
    estimated_states = simulate_trajectory(
        step, INITIAL_STATE, estimated_params, N_STEPS
    )

    final_loss, final_grad = grad_fun(ca.DM(estimated_params))
    final_loss = float(final_loss)
    final_grad = np.asarray(final_grad.full()).reshape(-1)

    parameter_path = build_history_points(INITIAL_GUESS, iterates, estimated_params)
    loss_history, grad_norm_history = evaluate_history(grad_fun, parameter_path)

    save_outputs(
        results_dir,
        t_grid,
        true_states,
        initial_guess_states,
        estimated_states,
        loss_history,
        grad_norm_history,
        TRUE_PARAMS,
        INITIAL_GUESS,
        estimated_params,
    )
    save_plots(
        results_dir,
        t_grid,
        true_states,
        initial_guess_states,
        estimated_states,
        loss_history,
        grad_norm_history,
    )

    print("Mass-spring-damper parameter estimation complete.")
    print(f"  True parameters:      k={TRUE_PARAMS[0]:.6f}, c={TRUE_PARAMS[1]:.6f}")
    print(f"  Initial guess:        k={INITIAL_GUESS[0]:.6f}, c={INITIAL_GUESS[1]:.6f}")
    print(
        f"  Estimated parameters:  k={estimated_params[0]:.6f}, c={estimated_params[1]:.6f}"
    )
    print(f"  Initial symbolic loss: {initial_loss_symbolic:.12e}")
    print(f"  Initial adjoint loss:  {initial_loss_adjoint:.12e}")
    print(f"  Final loss:            {final_loss:.12e}")
    print(f"  Final gradient norm:   {np.linalg.norm(final_grad):.12e}")
    print(f"  Outputs saved to:      {results_dir}")


if __name__ == "__main__":
    main()
