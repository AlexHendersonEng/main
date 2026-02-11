# C++

## Conan Instructions

1. Detect profile

    ```bash
    conan profile detect --force
    ```

1. Install

    ```bash
    conan install . --output-folder=build --build=missing
    ```

    To generate .ps1 files required for PowerShell add `--conf tools.env.virtualenv:powershell=pwsh`

1. Activate Conan environment

    ```bash
    conanbuild.(bat|ps1|sh)
    ```

1. Configure

    ```bash
    cmake --preset conan-default
    ```
