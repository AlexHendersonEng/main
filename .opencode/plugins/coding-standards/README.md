# Coding standards plugin for OpenCode

This plugin exposes the shared coding standards in `agent-marketplace/plugins/coding-standards/skills` to OpenCode. It
registers every skill with OpenCode's native `skill` tool and adds a `coding_standards` tool that returns the rules
that apply to a file or a named topic.

## Requirements

- OpenCode v2 (tested with v2.1.4)

## Use in this repository

OpenCode discovers plugins in `.opencode/plugins/` automatically, so there is nothing else to configure. The plugin
finds the shared skills in `agent-marketplace/plugins/coding-standards/skills` relative to the project root.

## Use in another project

Copy this directory to `<project>/.opencode/plugins/coding-standards/`, then do one of the following:

- Copy `agent-marketplace/plugins/coding-standards/skills` to `<project>/.opencode/skills/`.
- Copy it to `<project>/.opencode/plugins/coding-standards/skills/`.

OpenCode already discovers `.opencode/skills/`, and the plugin also checks both locations.

## Use in all projects

Copy this directory to `~/.config/opencode/plugins/coding-standards/`, then copy the shared skills to either
`~/.config/opencode/skills/` or `~/.config/opencode/plugins/coding-standards/skills/`.

## Tools

`coding_standards` takes either a `filePath` or a `topic`:

- `filePath` returns the global rules plus the language reference for that file type.
- `topic` returns one language reference (`global`, `cpp`, `python`, `web` or `markdown`) or one workflow skill
  (`format-and-lint`, `add-tests`, `restructure-modules`, `coding-standards`, `document-code` or `review-standards`).

The skills are also available directly through OpenCode's `skill` tool.
