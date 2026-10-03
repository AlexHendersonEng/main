# findr

Find a regular expression in directory names, file names, or file contents, and replace text in files.

## Install

```
cargo install --path .
```

## Usage

```
findr dirs    <PATTERN> [PATH...]                  # match directory names
findr files   <PATTERN> [PATH...]                  # match file names
findr text    <PATTERN> [PATH...]                  # match file contents (path:line:col: text)
findr replace <PATTERN> <REPLACEMENT> [PATH...]    # preview replacements; add --write to apply
```

`PATH` defaults to the current directory. Names are matched against the entry's own name, not its full path.

### Options

| Flag | Description |
|------|-------------|
| `-i`, `--ignore-case` | Case-insensitive matching |
| `-F`, `--fixed-strings` | Treat the pattern (and replacement) as literal text |
| `-g`, `--glob <GLOB>` | Include paths matching the glob; prefix with `!` to exclude. Repeatable |
| `--hidden` | Include hidden files and directories |
| `--no-ignore` | Do not respect `.gitignore` files |
| `-d`, `--max-depth <N>` | Maximum directory depth |
| `--color <auto\|always\|never>` | When to use color (default `auto`) |
| `--write` | (`replace` only) Apply changes instead of previewing |

### Examples

```
findr files '\.rs$' src                 # Rust files under src
findr dirs '^test' --hidden             # directories starting with "test"
findr text -i 'todo|fixme' -g '*.rs'    # search Rust sources
findr replace '(\w+)@(\w+)' '$2@$1'     # dry run: shows a diff, changes nothing
findr replace 'foo' 'bar' --write       # apply the replacement
```

## Behaviour notes

- Patterns use the [Rust regex syntax](https://docs.rs/regex/latest/regex/#syntax). In `replace`, `$1` and `${name}` refer to capture groups (use `$$` for a literal `$`).
- `replace` never writes without `--write`. Writes are atomic (temp file + rename) and keep file permissions.
- Binary files (containing a NUL byte in the first 8 KB) are skipped. In `replace`, files that are not valid UTF-8 are also skipped.
- Renaming files or directories is not supported.

## Exit codes

| Code | Meaning |
|------|---------|
| 0 | At least one match (or, for `replace`, at least one file changed/would change) |
| 1 | No matches |
| 2 | Error (e.g. invalid pattern) |
