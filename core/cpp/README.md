# C++

## Conan Instructions

Detect profile

```bash
conan profile detect --force
```

Install

```bash
conan install . --output-folder=build --build=missing
```

Configure

```bash
cmake --preset conan-default
```
