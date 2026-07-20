# Panel Methods

## Overview

This package implements panel method utilities and examples for potential flow aerodynamics and related educational material. It includes reference implementations of source, vortex and source-vortex panel methods, helper code for generating NACA airfoils, example notebooks and LaTeX documentation in `docs`.

## Prerequisites

- Astral's uv
- For building PDF docs: a TeX distribution (e.g., TeX Live, MiKTeX) `pdflatex`

## Install

To setup the virtual environment with uv,

```bash
uv sync
```

## Examples

The examples in this project can be found in the scripts directory and take the form of Jupyter notebooks.

## Building documentation

The LaTeX sources are under `docs/articles/`. Build a PDF for a given article with `latexmk` or `pdflatex`:

```bash
cd docs/articles/elementary_flows
pdflatex main.tex
```

## License

This project follows the repository license; see [LICENSE](../LICENSE).
