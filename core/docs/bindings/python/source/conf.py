import tomllib
from pathlib import Path

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

pyproj_path = Path(__file__).parent / ".." / ".." / ".." / ".." / "pyproject.toml"
with open(pyproj_path, "rb") as f:
    data = tomllib.load(f)

project = data["project"]["name"]
copyright = f"2026, {data['project']['authors'][0]['name']}"
author = data["project"]["authors"][0]["name"]
release = data["project"]["version"]

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "myst_parser",
    "sphinx.ext.autodoc",
    "sphinx.ext.napoleon",
    "sphinx_rtd_theme",
    "sphinx_design",
]
source_suffix = {
    ".rst": "restructuredtext",
    ".txt": "markdown",
    ".md": "markdown",
}
myst_enable_extensions = ["colon_fence"]
templates_path = ["_templates"]
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "sphinx_rtd_theme"
html_static_path = ["_static"]
