---
name: review-standards
description: >-
  Audit files or a diff against the personal coding standards and report violations without editing anything. Use
  whenever the user asks to review, check, audit or lint code against their standards, or before a commit or pull
  request.
---

# Review standards

Report only. Do not edit files unless the user asks.

1. Collect the files: the paths given, otherwise the changed files from `git diff --name-only HEAD`.
2. Read `../coding-standards/references/global.md` and the language reference for each file type.
3. Check every rule: blank lines around conditionals, loops and `return`, contiguous imports, a one-line documentation
   comment on every function, method, class and struct, line comments, `_test` file names, folder structure, and the
   language-specific rules.
4. Report violations grouped by rule as `path:line - problem - suggested fix`, then a short summary with counts.
5. Offer to fix them.
