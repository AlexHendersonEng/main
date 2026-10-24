---
name: coding-standards
description: >-
  Personal coding standards covering formatting, documentation comments, British English, file structure, libraries,
  testing and tooling for C++, Python, JavaScript/TypeScript, web markup and Markdown. Use this skill whenever writing,
  editing, refactoring or reviewing any code or Markdown file, even for small changes, and before finishing any
  coding task.
---

# Coding standards

Apply these standards to every change you make.

## Workflow

1. Read `references/global.md`. It applies to every file.
2. Read the language reference that matches each file you touch:

   | Files | Reference |
   |---|---|
   | `.cpp .cc .cxx .h .hpp .hh .hxx .ipp .inl .tpp .ixx .cppm` | `references/cpp.md` |
   | `.py` | `references/python.md` |
   | `.js .jsx .ts .tsx .mjs .cjs .html .css .scss .vue .svelte` | `references/web.md` |
   | `.md` | `references/markdown.md` |

3. Make the change, following the rules.
4. Run the project's configured formatter on the changed files as the final editing step (see the `format-and-lint`
   skill).

## When unsure

Ask rather than guess, at any stage. List the options with their trade-offs and say which one you recommend and why.

## Related skills

- `document-code` adds documentation and line comments.
- `add-tests` creates `_test` files and checks they are discovered.
- `restructure-modules` splits code into folders with an umbrella entry point.
- `review-standards` audits files against these rules without editing them.
- `format-and-lint` runs the project's own tooling.
- `install-instructions` installs these rules as native always-on instruction files.
