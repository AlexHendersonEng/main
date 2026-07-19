import numpy as np
from panel_methods.panel.panel import Panel
from typing import List
from panel_methods.elementary_flows.uniform_flow import UniformFlow
from panel_methods.panel.common import integration_term


class SourceVortexPanelSystem:
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
            np.zeros([self.n_panels + 1, self.n_panels + 1]),
            np.zeros([self.n_panels, self.n_panels + 1]),
        )
        bn, bt = np.zeros(self.n_panels + 1), np.zeros(self.n_panels)
        an_vortex, at_vortex = (
            np.zeros([self.n_panels, self.n_panels]),
            np.zeros([self.n_panels, self.n_panels]),
        )

        # Populate matrices
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
                    at_vortex[i, j] = 0.5
                    continue

                # Common terms for panel j
                phi_j = self.panels[j].phi
                x_j1 = self.panels[j].x1
                y_j1 = self.panels[j].y1
                s_j = self.panels[j].s

                # Geometric coefficients common to normal and tangential source terms
                a1_source = -(x_ic - x_j1) * np.cos(phi_j) - (y_ic - y_j1) * np.sin(
                    phi_j
                )
                a2_source = (x_ic - x_j1) ** 2 + (y_ic - y_j1) ** 2
                a5_source = np.sqrt(a2_source - a1_source**2)

                # normal influence source coefficients
                a3n_source = np.sin(phi_i - phi_j)
                a4n_source = -(x_ic - x_j1) * np.sin(phi_i) + (y_ic - y_j1) * np.cos(
                    phi_i
                )
                an[i, j] = integration_term(
                    a1_source, a2_source, a3n_source, a4n_source, a5_source, s_j
                )

                # Tangential influence source coefficients
                a3t_source = -np.cos(phi_i - phi_j)
                a4t_source = (x_ic - x_j1) * np.cos(phi_i) + (y_ic - y_j1) * np.sin(
                    phi_i
                )
                at[i, j] = integration_term(
                    a1_source, a2_source, a3t_source, a4t_source, a5_source, s_j
                )

                # Geometric coefficients common to normal and tangential vortex terms
                a1_vortex = -(x_ic - x_j1) * np.cos(phi_j) - (y_ic - y_j1) * np.sin(
                    phi_j
                )
                a2_vortex = (x_ic - x_j1) ** 2 + (y_ic - y_j1) ** 2
                a5_vortex = np.sqrt(a2_vortex - a1_vortex**2)

                # Normal influence vortex terms
                a3n_vortex = -np.cos(phi_i - phi_j)
                a4n_vortex = (x_ic - x_j1) * np.cos(phi_i) + (y_ic - y_j1) * np.sin(
                    phi_i
                )
                an_vortex[i, j] = -integration_term(
                    a1_vortex, a2_vortex, a3n_vortex, a4n_vortex, a5_vortex, s_j
                )

                # Tangential influence vortex terms
                a3t_vortex = np.sin(phi_j - phi_i)
                a4t_vortex = (x_ic - x_j1) * np.sin(phi_i) - (y_ic - y_j1) * np.cos(
                    phi_i
                )
                at_vortex[i, j] = -integration_term(
                    a1_vortex, a2_vortex, a3t_vortex, a4t_vortex, a5_vortex, s_j
                )

            # Normal and tangential influence contributions from the vortex panels
            an[i, self.n_panels] = sum(an_vortex[i, :])
            at[i, self.n_panels] = sum(at_vortex[i, :])

            # Freestream contributions to the RHS
            bn[i] = -self.uniform_flow.u_inf * np.cos(beta_i)
            bt[i] = -self.uniform_flow.u_inf * np.sin(beta_i)

        # Enforce Kutta condition
        bn[self.n_panels] = -self.uniform_flow.u_inf * (
            np.sin(self.panels[0].beta) + np.sin(self.panels[self.n_panels - 1].beta)
        )

        for j in range(self.n_panels):
            an[self.n_panels, j] = at[0, j] + at[self.n_panels - 1, j]
        an[self.n_panels, self.n_panels] = sum(
            at_vortex[0, :] + at_vortex[self.n_panels - 1, :]
        )

        # Compute result array
        self.strengths = np.linalg.solve(an, bn)

        # Evaluate surface normal and tangential velocities and pressure coefficient
        self.vn = (an @ self.strengths - bn)[:-1]
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
        ax, ay = np.zeros((n, self.n_panels + 1)), np.zeros((n, self.n_panels + 1))
        bx, by = (
            np.full(n, -self.uniform_flow.u_inf * np.cos(self.uniform_flow.alpha)),
            np.full(n, -self.uniform_flow.u_inf * np.sin(self.uniform_flow.alpha)),
        )
        ax_vortex, ay_vortex = (
            np.zeros([n, self.n_panels]),
            np.zeros([n, self.n_panels]),
        )

        # Populate influence coefficient matrices
        for i in range(n):
            # Common terms for field point i
            x_p = x_flat[i]
            y_p = y_flat[i]

            for j in range(self.n_panels):
                # Common terms for panel j
                phi_j = self.panels[j].phi
                x_j1 = self.panels[j].x1
                y_j1 = self.panels[j].y1
                s_j = self.panels[j].s

                # Geometric coefficients common to x and y direction source terms
                a1_source = -(x_p - x_j1) * np.cos(phi_j) - (y_p - y_j1) * np.sin(phi_j)
                a2_source = (x_p - x_j1) ** 2 + (y_p - y_j1) ** 2
                a5_source = np.sqrt(a2_source - a1_source**2)

                # x direction influence source coefficients
                a3x_source = -np.cos(phi_j)
                a4x_source = x_p - x_j1
                ax[i, j] = integration_term(
                    a1_source, a2_source, a3x_source, a4x_source, a5_source, s_j
                )

                # y direction influence source coefficients
                a3y_source = -np.sin(phi_j)
                a4y_source = y_p - y_j1
                ay[i, j] = integration_term(
                    a1_source, a2_source, a3y_source, a4y_source, a5_source, s_j
                )

                # Geometric coefficients common to x and y direction vortex terms
                a1_vortex = -(x_p - x_j1) * np.cos(phi_j) - (y_p - y_j1) * np.sin(phi_j)
                a2_vortex = (x_p - x_j1) ** 2 + (y_p - y_j1) ** 2
                a5_vortex = np.sqrt(a2_vortex - a1_vortex**2)

                # x direction influence vortex terms
                a3x_vortex = np.sin(phi_j)
                a4x_vortex = -(y_p - y_j1)
                ax_vortex[i, j] = -integration_term(
                    a1_vortex, a2_vortex, a3x_vortex, a4x_vortex, a5_vortex, s_j
                )

                # y direction influence vortex terms
                a3y_vortex = -np.cos(phi_j)
                a4y_vortex = x_p - x_j1
                ay_vortex[i, j] = -integration_term(
                    a1_vortex, a2_vortex, a3y_vortex, a4y_vortex, a5_vortex, s_j
                )

            # Vortex influence on x and y direction
            ax[i, self.n_panels] = sum(ax_vortex[i, :])
            ay[i, self.n_panels] = sum(ay_vortex[i, :])

        # Assemble velocity components and restore original array shape
        u = (ax @ self.strengths - bx).reshape(original_shape)
        v = (ay @ self.strengths - by).reshape(original_shape)

        return u, v
