import numpy as np


def integration_term(
    a1: float, a2: float, a3: float, a4: float, a5: float, s: float
) -> float:
    """
    Evaluates the analytic integral of a source panel's influence over its length.

    Parameters
    ----------
    a1: float
        Geometric coefficient derived from panel and field-point coordinates.
    a2: float
        Geometric coefficient equal to the squared distance from the field point
        to the panel start.
    a3: float
        Trigonometric coefficient for the logarithmic term.
    a4: float
        Trigonometric coefficient for the arctangent term.
    a5: float
        Geometric coefficient equal to the perpendicular distance from the field
        point to the panel line.
    s: float
        Panel length.

    Returns
    -------
    float
        Value of the integrated influence coefficient.
    """
    if a5 == 0 or np.iscomplex(a5) or np.isnan(a5) or np.isinf(a5):
        return 0.0

    log_term = 0.5 * a3 * np.log((s**2 + 2 * a1 * s + a2) / a2)
    atan_term = ((a4 - a1 * a3) / a5) * (np.atan2(s + a1, a5) - np.atan2(a1, a5))
    value = (log_term + atan_term) / (2 * np.pi)

    if np.iscomplex(value) or np.isnan(value) or np.isinf(value):
        return 0.0

    return value
