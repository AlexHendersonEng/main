## Asking questions

- Whenever you are unsure how to proceed with a task, or what the best way to proceed is, ask questions rather than
  guessing. Ask as many as needed, at any stage of the task: before starting, during implementation and before
  finishing.
- When asking, list the available options with their trade-offs, and say which one you recommend and why.

## Formatting

- Put a blank line before and after every conditional block (`if`/`else`/`switch`) and every loop
  (`for`/`while`/`do`), unless it is the first or last statement in its scope. Keep connected branches together.
- Put a blank line before every `return` statement, unless it is the only statement in its block.
- Do not leave blank lines between imports at the top of a file; keep them as one contiguous block.

## Documentation and comments

- Add a short, one-line documentation comment to every function, method, class and struct, using the language's
  standard documentation syntax. Document a declaration rather than duplicating its description at the definition.
- Keep comments concise. They may span multiple lines when needed, but should not be verbose.
- Every line of code should be covered by a comment.

## Language

- Always use British spelling and grammar, not American English, in names, comments, documentation and messages
  (e.g. `centre`, `colour`, `analyse`, `normalise`). Names imposed by the standard library, language standards or
  third-party APIs are the only exception.

## Structure

- Separate code into logical folders and files whenever that makes it cleaner and easier to read; prefer one main
  class, type or concept per file over large files holding many unrelated ones.
- Group related files in a folder named after the concept (e.g. `sections/circle.hpp`, `sections/ellipse.hpp`), and
  provide a single entry point (e.g. an umbrella header such as `sections.hpp` or a package `__init__.py`) so
  callers can import the whole group at once.

## Libraries

- Prefer a clean well-maintained library implementation over writing an algorithm from scratch, but only when it
  makes the code simpler.
- Reuse libraries the project already depends on before adding new ones, and add any new dependency through the
  project's existing package manager.

## Testing

- Name test files with a `_test` suffix before the extension (e.g. `geometry_test.cpp`), not a `test_` prefix. Ensure
  the existing build and test configuration discovers these names.

## Tooling

- After making all changes, run the project's configured formatter on the changed files as the final editing step.
- Use the project's existing build, linting, static-analysis, type-checking and testing commands. Do not introduce a
  different build system, testing framework, formatter or package manager solely to enforce these instructions.
