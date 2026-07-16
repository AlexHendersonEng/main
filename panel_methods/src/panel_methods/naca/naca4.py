import numpy as np


def naca4(
    designation: str, n_points: int = 100, blunt_te: bool = False
) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    """
    Generate the coordinates of a NACA 4-digit aerofoil.

    Parameters
    ----------
    designation : str
        4-digit NACA designation, e.g. '2412', '0012', '4415'.
    n_points : int, optional
        Number of points along each surface (upper and lower).
    blunt_te : bool, optional
        If True, the trailing edge is left open (blunt/finite thickness).
        If False, the trailing edge is forced closed (sharp). Default is False.

    Returns
    -------
    x_upper : np.ndarray
        x-coordinates running from leading edge to trailing edge for the upper surface,
        normalised to chord length (0 to 1).
    y_upper : np.ndarray
        Corresponding y-coordinates for the upper surface, normalised to chord length.
    x_lower : np.ndarray
        x-coordinates for the lower surface, normalised to chord length (0 to 1).
    y_lower : np.ndarray
        Corresponding y-coordinates for the lower surface, normalised to chord length.
    """
    if len(designation) != 4 or not designation.isdigit():
        raise ValueError(f"designation must be a 4-digit string, got '{designation}'.")

    m = int(designation[0]) / 100.0  # max camber
    p = int(designation[1]) / 10.0  # location of max camber
    t = int(designation[2:]) / 100.0  # max thickness

    # Cosine spacing for better resolution near LE and TE
    beta = np.linspace(0, np.pi, n_points)
    x_c = 0.5 * (1 - np.cos(beta))

    # Thickness distribution
    a4 = -0.1015 if blunt_te else -0.1036
    y_t = (
        5
        * t
        * (
            0.2969 * np.sqrt(x_c)
            - 0.1260 * x_c
            - 0.3516 * x_c**2
            + 0.2843 * x_c**3
            + a4 * x_c**4
        )
    )

    # Camber line and gradient
    y_c = np.zeros_like(x_c)
    dy_c = np.zeros_like(x_c)

    if m > 0 and p > 0:
        fwd = x_c < p
        aft = ~fwd

        y_c[fwd] = (m / p**2) * (2 * p * x_c[fwd] - x_c[fwd] ** 2)
        dy_c[fwd] = (m / p**2) * 2 * (p - x_c[fwd])

        y_c[aft] = (m / (1 - p) ** 2) * ((1 - 2 * p) + 2 * p * x_c[aft] - x_c[aft] ** 2)
        dy_c[aft] = (m / (1 - p) ** 2) * 2 * (p - x_c[aft])

    theta = np.arctan(dy_c)

    # Surface coordinates
    x_upper = x_c - y_t * np.sin(theta)
    y_upper = y_c + y_t * np.cos(theta)

    x_lower = x_c + y_t * np.sin(theta)
    y_lower = y_c - y_t * np.cos(theta)

    return x_upper, y_upper, x_lower, y_lower
