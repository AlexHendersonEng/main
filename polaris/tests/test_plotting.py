import numpy as np
import pytest

from polaris import Result
from polaris.plotting import compare, plot


@pytest.fixture
def result():
    t = np.linspace(0, 1, 5)
    return Result(t, {"x": t, "y": 2 * t})


def test_plot_single_axes(result, tmp_path):
    fig = plot(result, title="t", save_to=tmp_path / "a.png")
    assert len(fig.axes) == 1 and len(fig.axes[0].lines) == 2
    assert (tmp_path / "a.png").stat().st_size > 0


def test_plot_separate_and_selection(result):
    fig = plot(result, ["y"], separate=True)
    assert len(fig.axes) == 1 and fig.axes[0].get_ylabel() == "y"
    assert len(plot(result, separate=True).axes) == 2


def test_plot_errors():
    with pytest.raises(ValueError, match="no variables"):
        plot(Result(np.zeros(2), {}))
    with pytest.raises(KeyError):
        plot(Result(np.zeros(2), {"x": np.zeros(2)}), ["nope"])


def test_compare_and_method(result):
    fig = compare({"a": result, "b": result}, "x")
    assert [ln.get_label() for ln in fig.axes[0].lines] == ["a", "b"]
    with pytest.raises(ValueError):
        compare({}, "x")
    assert len(result.plot(["x"]).axes[0].lines) == 1
