---
name: restructure-modules
description: >-
  Split large or mixed files into a clean folder and file layout with one main concept per file and a single umbrella
  entry point. Use whenever a file holds many unrelated classes or types, the user asks to reorganise, split or
  modularise code, or a new group of related files is being added.
---

# Restructure modules

1. Identify the distinct concepts in the code. Prefer one main class, type or concept per file.
2. Group related files in a folder named after the concept, for example `sections/circle.hpp` and
   `sections/ellipse.hpp`.
3. Add one entry point per group so callers can import the whole group at once:
   - C++: an umbrella header such as `sections.hpp` that includes every header in the folder.
   - Python: a package `__init__.py` that imports and re-exports the public names.
   - TypeScript or JavaScript: an `index.ts` that re-exports the group.
4. Update every include, import and build file (CMake, `pyproject.toml`, bundler config) that referenced the old layout.
5. Keep behaviour identical. Build and run the existing tests afterwards.
6. Ask before moving files across package or module boundaries, and list the options with a recommendation.
