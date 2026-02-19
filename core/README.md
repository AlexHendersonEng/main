# Simulation Foundry

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)
![uv](https://img.shields.io/badge/uv-%23DE5FE9.svg?style=for-the-badge&logo=uv&logoColor=white)

Simulation Foundry is a developer friendly toolkit for designing, running, and analysing numerical simulations. It combines a high performance C++ core for simulation engines with Python tooling for data analysis, visualisation and experiment automation.

## Overview

- **C++ core:** A modular framework and utilities for implementing and running performant simulation kernels, solvers and numerical experiments.
- **Python tools:** Lightweight analysis and visualisation helpers to post-process simulation output, run parameter sweeps and build reproducible experiments.
- **Examples & tests:** Ready to run examples and test suites to help you get started quickly and validate results.

## Prerequisites

- vcpkg for dependency management
- A supported C++ compiler (MSVC on Windows, clang or gcc on macOS/Linux)
- clang-format and clang-tidy for code formatting and static analysis
- Python 3.12 or newer.
- uv for dependency management.
- pre-commit for git hooks

## Project Layout

- `cpp/` — C++ source, CMake files, examples and tests.
- `python/` — Python packaging, analysis scripts, notebooks and tests.

## Key Features

- Clear separation between simulation engine and analysis tooling.
- Cross-platform CMake based build for the C++ components.
- Python package layout for easy installation and scripting.
- Example simulations and Jupyter notebooks demonstrating common workflows.

## Getting Started

1. Read the platform specific C++ setup and build instructions in the `cpp` folder: [cpp/README.md](cpp/README.md).
1. Install the Python dependencies and try the examples in the `python` folder: [python/README.md](python/README.md).

## Git Hooks

Install hooks locally:

```bash
pre-commit install
```

Run hooks on the whole repository:

```bash
pre-commit run --all-files
```

## License

This project is released under the terms in the `LICENSE` file: [LICENSE](LICENSE)
