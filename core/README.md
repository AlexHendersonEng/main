# Simulation Foundry Core

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)
![uv](https://img.shields.io/badge/uv-%23DE5FE9.svg?style=for-the-badge&logo=uv&logoColor=white)

Simulation Foundry Core is a developer friendly toolkit for designing, running, and analysing numerical simulations.

## Prerequisites

- A supported C++ compiler (MSVC on Windows, clang or gcc on macOS/Linux)
- vcpkg for dependency management
- Astral uv for Python package building
- clang-format and clang-tidy for code formatting and static analysis
- pre-commit for git hooks

## C/C++

### Build Instructions

1. Set vcpkg root environment variable (replace `<vcpkg-root>`):

    Linux:

    ```bash
    export VCPKG_ROOT="<vcpkg-root>"
    ```

    Windows (Powershell):

    ```Powershell
    $env:VCPKG_ROOT="<vcpkg-root>"
    ```

1. Build with CMake workflow presets (replace `<workflow-preset>`):

    ```bash
    cmake --workflow --preset "<workflow-preset>"
    ```

    Workflow presets can be listed with:

    ```bash
    cmake --workflow --list-presets
    ```

### Examples

After building, example binaries can be found under `out/build/<preset>/examples`.

### Testing

Testing is run using `ctest` and is run automatically when executing a `cmake` workflow.

### Packaging

Packaging is run using `cpack` and is run automatically when executing a `cmake` workflow.

### Documentation

Documentation is generated using `doxygen` and is run automatically when executing a `cmake` workflow.

## Python

### Build Instructions

1. Set vcpkg root environment variable (replace `<vcpkg-root>`):

    Linux:

    ```bash
    export VCPKG_ROOT="<vcpkg-root>"
    ```

    Windows (Powershell):

    ```Powershell
    $env:VCPKG_ROOT="<vcpkg-root>"
    ```

1. Build using uv:

    ```bash
    uv build
    ```

1. Generate stubs:

    ```bash
    uv run pybind11-stubgen simulation_foundry_core_py
    ```

### Examples

Before running the examples execute:

```bash
uv pip install -e .
```

Python examples can be found under `examples/bindings/python`.

### Testing

Testing can be run be executing 

```bash
uv run pytest
```

### Documentation

Generate Python bindings documentation using Sphinx:

```bash
uv run sphinx-build ./docs/bindings/python/source ./out/build/docs/bindings/python
```

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

This project is released under the terms in the `LICENSE` file: [LICENSE](https://github.com/AlexHendersonEng/simulation-foundry-core/blob/main/LICENSE)
