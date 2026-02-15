# C++

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
![Conan](https://img.shields.io/badge/conan-%236699CB.svg?style=for-the-badge&logo=conan&logoColor=white)

This folder contains the C++ core including the source, CMake configuration, examples and tests. The project uses Conan for dependency management and CMake presets for configuring and building.

## Prerequisites

- Conan (for dependency management)
- A supported C++ compiler (MSVC on Windows, clang or gcc on macOS/Linux)

## Build Instructions

1. Detect or create a Conan profile (first-time setup):

    ```bash
    conan profile detect --force
    ```

1. Install dependencies into a local `build` output folder:

    ```bash
    conan install . --output-folder=build --build=missing
    ```

    If you want generated PowerShell environment activation scripts, add the option:

    ```bash
    conan install . --output-folder=build --build=missing --conf tools.env.virtualenv:powershell=pwsh
    ```

1. Activate the Conan provided environment

    - On Windows (CMD):

        ```batch
        conanbuild.bat
        ```

    - On Windows (Powershell):

        ```Powershell
        conanbuild.ps1
        ```

    - On macOS/Linux:

        ```bash
        source conanbuild.sh
        ```

1. Configure with CMake presets

    ```bash
    cmake --preset "conan-default"
    ```

1. Build using the release preset

    ```bash
    cmake --build --preset "conan-release"
    ```

## Examples

After building, example binaries can be found under `build/examples`.

## Testing

After building, run unit tests via `ctest` from the build directory:

```bash
ctest --preset "conan-release"
```
