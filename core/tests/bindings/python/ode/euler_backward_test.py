import math
from simulation_foundry_core_py.ode import euler_backward


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


class TestEulerBackwardOutputStructure:
    def test_returns_solution(self):
        from simulation_foundry_core_py.ode import Solution

        sol = euler_backward(f=decay, t0=0.0, t1=1.0, y0=[1.0], h=0.1)
        assert isinstance(sol, Solution)

    def test_t_and_y_same_length(self):
        sol = euler_backward(f=decay, t0=0.0, t1=1.0, y0=[1.0], h=0.1)
        assert len(sol.t) == len(sol.y)

    def test_state_dimension_preserved(self):
        sol = euler_backward(f=harmonic, t0=0.0, t1=1.0, y0=[1.0, 0.0], h=0.1)
        assert all(len(row) == 2 for row in sol.y)

    def test_step_count(self):
        t0, t1, h = 0.0, 1.0, 0.1
        sol = euler_backward(f=decay, t0=t0, t1=t1, y0=[1.0], h=h)
        expected = math.ceil((t1 - t0) / h) + 1
        assert len(sol) == expected


class TestEulerBackwardTimeGrid:
    def test_starts_at_t0(self):
        sol = euler_backward(f=decay, t0=2.0, t1=3.0, y0=[1.0], h=0.1)
        assert math.isclose(sol.t[0], 2.0)

    def test_ends_at_t1(self):
        sol = euler_backward(f=decay, t0=0.0, t1=5.0, y0=[1.0], h=0.1)
        assert math.isclose(sol.t[-1], 5.0, abs_tol=1e-10)

    def test_time_is_monotone_increasing(self):
        sol = euler_backward(f=decay, t0=0.0, t1=2.0, y0=[1.0], h=0.1)
        assert all(a < b for a, b in zip(sol.t, sol.t[1:]))

    def test_single_step(self):
        sol = euler_backward(f=decay, t0=0.0, t1=0.1, y0=[1.0], h=0.1)
        assert len(sol) == 2


class TestEulerBackwardInitialCondition:
    def test_initial_state_preserved(self):
        sol = euler_backward(f=decay, t0=0.0, t1=1.0, y0=[3.7], h=0.1)
        assert math.isclose(sol.y[0][0], 3.7, rel_tol=1e-9)

    def test_initial_state_preserved_multi_dim(self):
        sol = euler_backward(f=harmonic, t0=0.0, t1=1.0, y0=[1.0, 2.0], h=0.1)
        assert math.isclose(sol.y[0][0], 1.0, rel_tol=1e-9)
        assert math.isclose(sol.y[0][1], 2.0, rel_tol=1e-9)


class TestEulerBackwardCorrectness:
    def test_zero_rhs_state_unchanged(self):
        sol = euler_backward(f=zero_rhs, t0=0.0, t1=1.0, y0=[42.0], h=0.1)
        assert all(math.isclose(row[0], 42.0, rel_tol=1e-12) for row in sol.y)

    def test_decay_is_monotone_decreasing(self):
        sol = euler_backward(f=decay, t0=0.0, t1=3.0, y0=[1.0], h=0.1)
        values = [row[0] for row in sol.y]
        assert all(a > b for a, b in zip(values, values[1:]))

    def test_decay_remains_positive(self):
        sol = euler_backward(f=decay, t0=0.0, t1=5.0, y0=[1.0], h=0.1)
        assert all(row[0] > 0 for row in sol.y)

    def test_linear_growth_accuracy(self):
        """dy/dt = 1 is exact for backward Euler (no truncation error)."""
        sol = euler_backward(f=linear_growth, t0=0.0, t1=1.0, y0=[0.0], h=0.1)
        assert math.isclose(sol.y[-1][0], 1.0, rel_tol=1e-9)

    def test_decay_accuracy(self):
        """First-order accuracy: small h should give reasonable agreement."""
        sol = euler_backward(f=decay, t0=0.0, t1=5.0, y0=[1.0], h=0.01)
        exact = math.exp(-5.0)
        assert math.isclose(sol.y[-1][0], exact, rel_tol=5e-2)

    def test_non_unit_initial_condition_scales_correctly(self):
        sol1 = euler_backward(f=decay, t0=0.0, t1=1.0, y0=[1.0], h=0.01)
        sol2 = euler_backward(f=decay, t0=0.0, t1=1.0, y0=[2.0], h=0.01)
        assert math.isclose(sol2.y[-1][0], 2.0 * sol1.y[-1][0], rel_tol=1e-6)

    def test_stiff_stability(self):
        """Backward Euler should remain stable on a stiff problem with large h
        where forward Euler would diverge."""
        sol = euler_backward(
            f=lambda t, y: [-100.0 * y[0]], t0=0.0, t1=1.0, y0=[1.0], h=0.1
        )
        # Solution must stay bounded and decay toward zero
        assert all(abs(row[0]) <= 1.0 for row in sol.y)
        assert sol.y[-1][0] >= 0.0
