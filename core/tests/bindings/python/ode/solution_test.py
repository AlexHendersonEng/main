import math
from simulation_foundry_core_py.ode import Solution


class TestSolutionInit:
    def test_default_construction(self):
        sol = Solution()
        assert sol is not None

    def test_repr_returns_string(self):
        assert isinstance(repr(Solution()), str)


class TestSolutionLen:
    def test_len_empty(self):
        sol = Solution()
        assert len(sol) == 0

    def test_len_after_assignment(self):
        sol = Solution()
        sol.t = [0.0, 0.1, 0.2]
        sol.y = [[1.0], [0.9], [0.8]]
        assert len(sol) == 3

    def test_len_single_point(self):
        sol = Solution()
        sol.t = [0.0]
        sol.y = [[1.0]]
        assert len(sol) == 1


class TestSolutionTProperty:
    def test_t_setter_and_getter(self):
        sol = Solution()
        sol.t = [0.0, 0.5, 1.0]
        assert list(sol.t) == [0.0, 0.5, 1.0]

    def test_t_single_value(self):
        sol = Solution()
        sol.t = [0.0]
        assert list(sol.t) == [0.0]

    def test_t_large_array(self):
        times = [i * 0.01 for i in range(1001)]
        sol = Solution()
        sol.t = times
        assert len(sol.t) == 1001
        assert math.isclose(sol.t[-1], 10.0)


class TestSolutionYProperty:
    def test_y_setter_and_getter_scalar_state(self):
        sol = Solution()
        sol.t = [0.0, 0.1]
        sol.y = [[1.0], [0.9]]
        assert math.isclose(sol.y[0][0], 1.0)
        assert math.isclose(sol.y[1][0], 0.9)

    def test_y_setter_and_getter_multi_state(self):
        sol = Solution()
        sol.t = [0.0, 0.1]
        sol.y = [[1.0, 0.0], [0.99, -0.1]]
        assert len(sol.y[0]) == 2
        assert len(sol.y[1]) == 2

    def test_y_values_preserved(self):
        states = [[float(i), float(-i)] for i in range(5)]
        sol = Solution()
        sol.t = [float(i) for i in range(5)]
        sol.y = states
        for i in range(5):
            assert math.isclose(sol.y[i][0], float(i))
            assert math.isclose(sol.y[i][1], float(-i))
