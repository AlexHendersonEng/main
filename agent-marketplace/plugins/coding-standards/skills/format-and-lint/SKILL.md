---
name: format-and-lint
description: >-
  Run the project's own configured formatters, linters and tests on the changed files as the final editing step. Use at
  the end of any coding task, before committing, or when the user asks to format, lint or tidy code.
---

# Format and lint

Use only the tools the project already configures. Never introduce a different formatter, linter, build system or
package manager.

1. List the changed files with `git diff --name-only HEAD` and `git ls-files --others --exclude-standard`.
2. Run the formatter for each file type, using the project's configuration:

   | Files | Formatter | Configured by |
   |---|---|---|
   | C++ | `clang-format -i <files>` | `.clang-format` |
   | Python | `ruff format <files>` | `pyproject.toml` or `ruff.toml` |
   | JavaScript, TypeScript, web | the project's formatter, e.g. `prettier --write <files>` | Prettier config |
   | Markdown | the project's formatter if any; keep lines to 120 characters | none required |

3. Python imports must stay one contiguous block. If ruff's `I001` rule is enabled, set
   `[tool.ruff.lint.isort] no-lines-before` to every section so `ruff check --fix` does not add blank lines.
4. Run the project's existing lint, type-check and test commands where they exist, and report failures.
5. If a formatter is not installed or not configured, say so and ask how to proceed rather than substituting another.
