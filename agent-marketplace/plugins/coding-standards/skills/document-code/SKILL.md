---
name: document-code
description: >-
  Add or fix documentation comments and line comments in code. Use whenever the user asks to document, comment or
  annotate code, or when new functions, methods, classes or structs have been added without documentation.
---

# Document code

Follow `../coding-standards/references/global.md` (Documentation and comments) and the language reference.

1. Add a one-line documentation comment to every function, method, class and struct:
   - C++: Doxygen (`/** @brief ... */` or `///`) when the project uses Doxygen, otherwise the project's comment style.
   - Python: a one-line docstring.
   - JavaScript and TypeScript: a one-line JSDoc or TSDoc description (`/** ... */`).
2. Document the declaration, not the definition, when both exist (for example a C++ header and its source file).
3. Make sure every line of code is covered by a comment. A comment may cover a small group of closely related lines.
4. Keep comments concise and in British English.
5. Do not change behaviour. If a name or design looks wrong, mention it instead of altering it.
