# C++

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)

This folder contains the C++ core including the source, CMake configuration, examples and tests. The project uses vcpkg for dependency management and CMake presets for configuring and building.

## Build Instructions

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

## Examples

After building, example binaries can be found under `build/examples`.
