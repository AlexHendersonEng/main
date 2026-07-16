import numpy as np


class Panel:
    """
    Represents a single straight-line source panel defined by two endpoints.
    """

    def __init__(self, x1: float, y1: float, x2: float, y2: float):
        """
        Sets the endpoints of the panel and derives its geometric properties.

        Parameters
        ----------
        x1: float
            x-coordinate of the panel start point.
        y1: float
            y-coordinate of the panel start point.
        x2: float
            x-coordinate of the panel end point.
        y2: float
            y-coordinate of the panel end point.
        """
        # Panel endpoints and centre point
        self.x1, self.y1 = x1, y1
        self.x2, self.y2 = x2, y2
        self.xc, self.yc = (x1 + x2) / 2.0, (y1 + y2) / 2.0

        # Panel length
        self.s = ((self.x2 - self.x1) ** 2 + (self.y2 - self.y1) ** 2) ** 0.5

        # Panel orientation angles
        self.phi = np.atan2(y2 - y1, x2 - x1)  # Inclination angle
        if self.phi < 0:
            self.phi += 2 * np.pi
        self.delta = self.phi + np.pi / 2.0  # Normal angle
        self.beta = self.delta  # Angle relative to freestream

        # Outward unit normal components
        self.xn, self.yn = np.cos(self.delta), np.sin(self.delta)
