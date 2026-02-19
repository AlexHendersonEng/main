# Python

![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)
![uv](https://img.shields.io/badge/uv-%23DE5FE9.svg?style=for-the-badge&logo=uv&logoColor=white)

This folder contains Python tooling for analysing, visualising and automating experiments driven by the C++ simulation core. The layout uses a modern packaging approach (`pyproject.toml`) and includes example scripts and Jupyter notebooks to help you get started.

## Installing Dependencies

Using uv:

```bash
uv sync
```

## Examples

Example scripts and notebooks are in `python/scripts`.

## Testing

Run the Python test suite with `pytest`:

```bash
uv run pytest
```
