These rules extend the global instructions.

## Python formatting

- Treat `import` and `from` statements as the contiguous import block.
- Do not separate standard-library, third-party and local import groups with blank lines. Imports may still be sorted
  by group. When ruff's isort rule (`I001`) is enabled, set `[tool.ruff.lint.isort] no-lines-before` to every section
  so `ruff check --fix` keeps the block contiguous.

## Python documentation

- Use a one-line docstring for every function, method and class.

## Python language

- Names imposed by third-party APIs (e.g. matplotlib's `color`) are exempt from the British English rule.

## Python testing

- Example: `io_test.py`.

## Python tooling

- Run `ruff format` as the formatter.
