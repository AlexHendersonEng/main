import math
from simulation_foundry_core_py.ode import runge_kutta_4


# Helpers
def decay(t, y):
    """dy/dt = -y  →  exact: y(t) = exp(-t)"""
    return [-y[0]]


def harmonic(t, y):
    """y'' + y = 0  →  state: [position, velocity]"""
    return [y[1], -y[0]]


def zero_rhs(t, y):
    return [0.0] * len(y)


def linear_growth(t, y):
    """dy/dt = 1  →  exact: y(t) = y0 + t"""
    return [1.0]


def cubic(t, y):
    """dy/dt = t^2  →  exact: y(t) = y0 + t^3/3"""
    return [t**2]


class TestRungeKutta4OutputStructure:
    def test_returns_solution(self):
        from simulation_foundry_core_py.ode import Solution

        sol = runge_kutta_4(f=decay, t0=0.0, t1=1.0, y0=[1.0], h=0.1)
        assert isinstance(sol, Solution)

    def test_t_and_y_same_length(self):
        sol = runge_kutta_4(f=decay, t0=0.0, t1=1.0, y0=[1.0], h=0.1)
        assert len(sol.t) == len(sol.y)

    def test_state_dimension_preserved(self):
        sol = runge_kutta_4(f=harmonic, t0=0.0, t1=1.0, y0=[1.0, 0.0], h=0.1)
        assert all(len(row) == 2 for row in sol.y)

    def test_step_count(self):
        t0, t1, h = 0.0, 1.0, 0.1
        sol = runge_kutta_4(f=decay, t0=t0, t1=t1, y0=[1.0], h=h)
        expected = math.ceil((t1 - t0) / h) + 1
        assert len(sol) == expected


class TestRungeKutta4TimeGrid:
    def test_starts_at_t0(self):
        sol = runge_kutta_4(f=decay, t0=2.0, t1=3.0, y0=[1.0], h=0.1)
        assert math.isclose(sol.t[0], 2.0)

    def test_ends_at_t1(self):
        sol = runge_kutta_4(f=decay, t0=0.0, t1=5.0, y0=[1.0], h=0.1)
        assert math.isclose(sol.t[-1], 5.0, abs_tol=1e-10)

    def test_time_is_monotone_increasing(self):
        sol = runge_kutta_4(f=decay, t0=0.0, t1=2.0, y0=[1.0], h=0.1)
        assert all(a < b for a, b in zip(sol.t, sol.t[1:]))

    def test_single_step(self):
        sol = runge_kutta_4(f=decay, t0=0.0, t1=0.1, y0=[1.0], h=0.1)
        assert len(sol) == 2


class TestRungeKutta4InitialCondition:
    def test_initial_state_preserved(self):
        sol = runge_kutta_4(f=decay, t0=0.0, t1=1.0, y0=[3.7], h=0.1)
        assert math.isclose(sol.y[0][0], 3.7, rel_tol=1e-9)

    def test_initial_state_preserved_multi_dim(self):
        sol = runge_kutta_4(f=harmonic, t0=0.0, t1=1.0, y0=[1.0, 2.0], h=0.1)
        assert math.isclose(sol.y[0][0], 1.0, rel_tol=1e-9)
        assert math.isclose(sol.y[0][1], 2.0, rel_tol=1e-9)


class TestRungeKutta4Correctness:
    def test_zero_rhs_state_unchanged(self):
        sol = runge_kutta_4(f=zero_rhs, t0=0.0, t1=1.0, y0=[42.0], h=0.1)
        assert all(math.isclose(row[0], 42.0, rel_tol=1e-12) for row in sol.y)

    def test_decay_is_monotone_decreasing(self):
        sol = runge_kutta_4(f=decay, t0=0.0, t1=3.0, y0=[1.0], h=0.1)
        values = [row[0] for row in sol.y]
        assert all(a > b for a, b in zip(values, values[1:]))

    def test_decay_remains_positive(self):
        sol = runge_kutta_4(f=decay, t0=0.0, t1=5.0, y0=[1.0], h=0.1)
        assert all(row[0] > 0 for row in sol.y)

    def test_linear_growth_exact(self):
        """RK4 is exact for polynomials up to degree 3; dy/dt=1 is exact."""
        sol = runge_kutta_4(f=linear_growth, t0=0.0, t1=1.0, y0=[0.0], h=0.1)
        assert math.isclose(sol.y[-1][0], 1.0, rel_tol=1e-9)

    def test_cubic_rhs_exact(self):
        """dy/dt = t^2 is a degree-2 polynomial; RK4 should be exact."""
        sol = runge_kutta_4(f=cubic, t0=0.0, t1=1.0, y0=[0.0], h=0.1)
        exact = 1.0 / 3.0  # y(1) = 0 + 1^3/3
        assert math.isclose(sol.y[-1][0], exact, rel_tol=1e-9)

    def test_decay_high_accuracy(self):
        """Fourth-order accuracy: should match exact solution tightly."""
        sol = runge_kutta_4(f=decay, t0=0.0, t1=5.0, y0=[1.0], h=0.01)
        exact = math.exp(-5.0)
        assert math.isclose(sol.y[-1][0], exact, rel_tol=1e-6)

    def test_non_unit_initial_condition_scales_correctly(self):
        sol1 = runge_kutta_4(f=decay, t0=0.0, t1=1.0, y0=[1.0], h=0.01)
        sol2 = runge_kutta_4(f=decay, t0=0.0, t1=1.0, y0=[2.0], h=0.01)
        assert math.isclose(sol2.y[-1][0], 2.0 * sol1.y[-1][0], rel_tol=1e-9)

    def test_harmonic_oscillator_energy_conservation(self):
        """RK4 should approximately conserve E = 0.5*(x^2 + v^2)."""
        sol = runge_kutta_4(f=harmonic, t0=0.0, t1=10.0, y0=[1.0, 0.0], h=0.01)
        e0 = 0.5 * (sol.y[0][0] ** 2 + sol.y[0][1] ** 2)
        ef = 0.5 * (sol.y[-1][0] ** 2 + sol.y[-1][1] ** 2)
        assert math.isclose(e0, ef, rel_tol=1e-4)

    def test_harmonic_oscillator_position_accuracy(self):
        """x(t) = cos(t) for [x0, v0] = [1, 0]."""
        sol = runge_kutta_4(f=harmonic, t0=0.0, t1=2 * math.pi, y0=[1.0, 0.0], h=1e-3)
        assert math.isclose(sol.y[-1][0], 1.0, rel_tol=1e-5)  # cos(2π) = 1
        assert math.isclose(sol.y[-1][1], 0.0, abs_tol=1e-3)  # sin(2π) = 0
