---
name: add-tests
description: >-
  Create or update unit tests using the `_test` suffix naming convention and make sure the project's build and test
  configuration discovers them. Use whenever the user asks for tests, test coverage or a test file, or when new
  behaviour has been added without tests.
---

# Add tests

1. Detect the project's existing test framework and reuse it. Never introduce a new one.
2. Name test files with a `_test` suffix before the extension: `geometry_test.cpp`, `io_test.py`, `geometry_test.ts`,
   `viewer_test.tsx`. Never use a `test_` prefix.
3. Check that the existing configuration discovers the `_test` names, and update it only as far as needed:
   - pytest: `python_files = ["*_test.py"]` under `[tool.pytest.ini_options]`.
   - Jest or Vitest: a `testMatch` or `include` pattern such as `**/*_test.{ts,tsx}`.
   - CMake: the glob or `add_executable` list used for test targets (and `gtest_discover_tests` or
     `catch_discover_tests` when present).
4. Place each test file next to the code it covers, or in the project's existing test folder.
5. Follow `../coding-standards/references/global.md`, including documentation comments and line comments.
6. Run the project's test command and report the result.
