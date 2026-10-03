use crate::cli::ReplaceArgs;
use crate::output::color_choice;
use crate::text::is_binary;
use crate::walk::{build_regex, walk};
use anyhow::{Context, Result};
use regex::Regex;
use similar::{ChangeTag, TextDiff};
use std::io::Write;
use std::path::Path;
use termcolor::{Color, ColorSpec, StandardStream, WriteColor};

/// Previews (or with --write, applies) replacements. Returns the number of files changed.
pub fn run(args: &ReplaceArgs) -> Result<usize> {
    let re = build_regex(&args.pattern, &args.common)?;
    let mut out = StandardStream::stdout(color_choice(args.common.color));
    let mut changed = 0;
    for entry in walk(&args.common)? {
        let entry = match entry {
            Ok(e) => e,
            Err(e) => {
                eprintln!("findr: {e}");
                continue;
            }
        };
        if !entry.file_type().is_some_and(|t| t.is_file()) {
            continue;
        }
        match process_file(&re, args, entry.path(), &mut out) {
            Ok(true) => changed += 1,
            Ok(false) => {}
            Err(e) => eprintln!("findr: {}: {e:#}", entry.path().display()),
        }
    }
    let verb = if args.write {
        "changed"
    } else {
        "would change"
    };
    eprintln!(
        "{verb} {changed} file(s){}",
        if args.write {
            ""
        } else {
            " (use --write to apply)"
        }
    );
    Ok(changed)
}

/// Returns the replaced text, or None when the file has no matches.
/// Replacement is literal when `fixed_strings` is set, otherwise `$1`/`${name}` expand.
pub fn replace_text(re: &Regex, text: &str, replacement: &str, literal: bool) -> Option<String> {
    if !re.is_match(text) {
        return None;
    }
    let new = if literal {
        re.replace_all(text, regex::NoExpand(replacement))
    } else {
        re.replace_all(text, replacement)
    };
    Some(new.into_owned())
}

fn process_file(
    re: &Regex,
    args: &ReplaceArgs,
    path: &Path,
    out: &mut impl WriteColor,
) -> Result<bool> {
    let bytes = std::fs::read(path)?;
    if is_binary(&bytes) {
        return Ok(false);
    }
    // Non-UTF-8 files are skipped so a lossy round trip can never corrupt them.
    let Ok(old) = String::from_utf8(bytes) else {
        eprintln!("findr: {}: skipped (not valid UTF-8)", path.display());
        return Ok(false);
    };
    let Some(new) = replace_text(re, &old, &args.replacement, args.common.fixed_strings) else {
        return Ok(false);
    };
    if new == old {
        return Ok(false);
    }
    print_diff(path, &old, &new, out)?;
    if args.write {
        write_atomic(path, new.as_bytes())?;
    }
    Ok(true)
}

fn print_diff(path: &Path, old: &str, new: &str, out: &mut impl WriteColor) -> Result<()> {
    let diff = TextDiff::from_lines(old, new);
    out.set_color(ColorSpec::new().set_bold(true))?;
    writeln!(out, "--- {p}\n+++ {p}", p = path.display())?;
    out.reset()?;
    for group in diff.grouped_ops(3) {
        let (first, last) = (group.first().unwrap(), group.last().unwrap());
        let (o, n) = (
            first.old_range().start..last.old_range().end,
            first.new_range().start..last.new_range().end,
        );
        writeln!(
            out,
            "@@ -{},{} +{},{} @@",
            o.start + 1,
            o.len(),
            n.start + 1,
            n.len()
        )?;
        for op in &group {
            for change in diff.iter_changes(op) {
                let (sign, color) = match change.tag() {
                    ChangeTag::Delete => ('-', Some(Color::Red)),
                    ChangeTag::Insert => ('+', Some(Color::Green)),
                    ChangeTag::Equal => (' ', None),
                };
                out.set_color(ColorSpec::new().set_fg(color))?;
                write!(out, "{sign}{}", change.value())?;
                if change.missing_newline() {
                    writeln!(out)?;
                }
                out.reset()?;
            }
        }
    }
    Ok(())
}

/// Writes to a temp file in the same directory, copies permissions, then renames over the target.
fn write_atomic(path: &Path, data: &[u8]) -> Result<()> {
    let dir = path
        .parent()
        .filter(|p| !p.as_os_str().is_empty())
        .unwrap_or(Path::new("."));
    let mut tmp = tempfile::NamedTempFile::new_in(dir).context("creating temp file")?;
    tmp.write_all(data)?;
    tmp.as_file()
        .set_permissions(std::fs::metadata(path)?.permissions())?;
    tmp.persist(path)
        .map_err(|e| e.error)
        .context("replacing file")?;
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn expands_capture_groups() {
        let re = Regex::new(r"(\w+)@(\w+)").unwrap();
        assert_eq!(replace_text(&re, "a@b", "$2@$1", false).unwrap(), "b@a");
    }

    #[test]
    fn literal_replacement_does_not_expand() {
        let re = Regex::new("a").unwrap();
        assert_eq!(replace_text(&re, "a", "$1", true).unwrap(), "$1");
    }

    #[test]
    fn no_match_is_none() {
        let re = Regex::new("x").unwrap();
        assert!(replace_text(&re, "abc", "y", false).is_none());
    }
}
