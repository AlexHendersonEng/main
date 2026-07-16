import numpy as np
from panel_methods.panel.panel import Panel
from typing import List
from panel_methods.elementary_flows.uniform_flow import UniformFlow
from panel_methods.panel.common import integration_term


class SourcePanelSystem:
    """
    Assembles and solves a source panel method for a closed body in a uniform flow.
    """

    def __init__(self, panels: List[Panel], uniform_flow: UniformFlow):
        """
        Sets up the panel system and computes each panel's angle relative to the freestream.

        Parameters
        ----------
        panels: List[Panel]
            Ordered list of Panel objects discretising the body surface.
        uniform_flow: UniformFlow
            Freestream flow conditions.
        """
        # Panels and freestream
        self.panels = panels
        self.n_panels = len(panels)
        self.uniform_flow = uniform_flow

        # Angle of each panel normal relative to the freestream direction
        for panel in self.panels:
            panel.beta = panel.delta - self.uniform_flow.alpha
            if panel.beta > 2 * np.pi:
                panel.beta -= 2 * np.pi

        # Solution quantities (populated after solve)
        self.strengths = None
        self.vn = None
        self.vt = None
        self.cp = None

    def solve(self):
        """
        Solves for panel source strengths and computes surface velocities and
        pressure coefficients.
        """
        # Preallocate influence coefficient and RHS matrices
        an, at = (
            np.zeros((self.n_panels, self.n_panels)),
            np.zeros((self.n_panels, self.n_panels)),
        )
        bn, bt = np.zeros(self.n_panels), np.zeros(self.n_panels)

        # Populate influence coefficient and RHS matrices
        for i in range(self.n_panels):
            # Common terms for panel i
            phi_i = self.panels[i].phi
            beta_i = self.panels[i].beta
            x_ic = self.panels[i].xc
            y_ic = self.panels[i].yc

            for j in range(self.n_panels):
                # Diagonal terms
                if i == j:
                    an[i, j] = 0.5
                    continue

                # Geometric coefficients common to normal and tangential terms
                phi_j = self.panels[j].phi
                x_j1 = self.panels[j].x1
                y_j1 = self.panels[j].y1
                s_j = self.panels[j].s
                a1 = -(x_ic - x_j1) * np.cos(phi_j) - (y_ic - y_j1) * np.sin(phi_j)
                a2 = (x_ic - x_j1) ** 2 + (y_ic - y_j1) ** 2
                a5 = np.sqrt(a2 - a1**2)

                # Normal influence coefficient
                a3n = np.sin(phi_i - phi_j)
                a4n = -(x_ic - x_j1) * np.sin(phi_i) + (y_ic - y_j1) * np.cos(phi_i)
                an[i, j] = integration_term(a1, a2, a3n, a4n, a5, s_j)

                # Tangential influence coefficient
                a3t = -np.cos(phi_i - phi_j)
                a4t = (x_ic - x_j1) * np.cos(phi_i) + (y_ic - y_j1) * np.sin(phi_i)
                at[i, j] = integration_term(a1, a2, a3t, a4t, a5, s_j)

            # Freestream contributions to the RHS
            bn[i] = -self.uniform_flow.u_inf * np.cos(beta_i)
            bt[i] = -self.uniform_flow.u_inf * np.sin(beta_i)

        # Solve for panel strengths
        self.strengths = np.linalg.solve(an, bn)

        # Evaluate surface normal and tangential velocities and pressure coefficient
        self.vn = an @ self.strengths - bn
        self.vt = at @ self.strengths - bt
        self.cp = 1.0 - (self.vt / self.uniform_flow.u_inf) ** 2

    def velocity(self, x: np.ndarray, y: np.ndarray):
        """
        Computes the velocity field at arbitrary points in the flow domain.

        Parameters
        ----------
        x: 2D Numpy array of floats
            x-coordinates of the field points.
        y: 2D Numpy array of floats
            y-coordinates of the field points.

        Returns
        -------
        u: 2D Numpy array of floats
            x-component of the velocity field.
        v: 2D Numpy array of floats
            y-component of the velocity field.
        """
        # Flatten to 1D for matrix assembly, retaining original shape for output
        original_shape = x.shape
        x_flat = x.ravel()
        y_flat = y.ravel()

        # Preallocate influence coefficient and freestream RHS arrays
        n = x_flat.size
        ax, ay = np.zeros((n, self.n_panels)), np.zeros((n, self.n_panels))
        bx, by = (
            np.full(n, -self.uniform_flow.u_inf * np.cos(self.uniform_flow.alpha)),
            np.full(n, -self.uniform_flow.u_inf * np.sin(self.uniform_flow.alpha)),
        )

        # Populate influence coefficient matrices
        for i in range(n):
            # Common terms for field point i
            x_p = x_flat[i]
            y_p = y_flat[i]

            for j in range(self.n_panels):
                # Geometric coefficients common to x and y terms
                phi_j = self.panels[j].phi
                x_j1 = self.panels[j].x1
                y_j1 = self.panels[j].y1
                s_j = self.panels[j].s
                a1 = -(x_p - x_j1) * np.cos(phi_j) - (y_p - y_j1) * np.sin(phi_j)
                a2 = (x_p - x_j1) ** 2 + (y_p - y_j1) ** 2
                a5 = (a2 - a1**2) ** 0.5

                # x-direction influence coefficient
                a3x = -np.cos(phi_j)
                a4x = x_p - x_j1
                ax[i, j] = integration_term(a1, a2, a3x, a4x, a5, s_j)

                # y-direction influence coefficient
                a3y = -np.sin(phi_j)
                a4y = y_p - y_j1
                ay[i, j] = integration_term(a1, a2, a3y, a4y, a5, s_j)

        # Assemble velocity components and restore original array shape
        u = (ax @ self.strengths - bx).reshape(original_shape)
        v = (ay @ self.strengths - by).reshape(original_shape)

        return u, v
