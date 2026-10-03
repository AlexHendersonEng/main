use crate::cli::{CommonArgs, SearchArgs};
use crate::output::color_choice;
use crate::walk::{build_regex, walk};
use anyhow::Result;
use regex::Regex;
use std::path::Path;
use termcolor::{Color, ColorSpec, StandardStream, WriteColor};

const BINARY_SNIFF_LEN: usize = 8192;

pub fn is_binary(bytes: &[u8]) -> bool {
    bytes[..bytes.len().min(BINARY_SNIFF_LEN)].contains(&0)
}

/// Prints `path:line:col: text` for each matching line. Returns the number of matching lines.
pub fn run(args: &SearchArgs) -> Result<usize> {
    let re = build_regex(&args.pattern, &args.common)?;
    let mut out = StandardStream::stdout(color_choice(args.common.color));
    search(&re, &args.common, &mut out)
}

pub fn search(re: &Regex, common: &CommonArgs, out: &mut impl WriteColor) -> Result<usize> {
    let mut count = 0;
    for entry in walk(common)? {
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
        match search_file(re, entry.path(), out) {
            Ok(n) => count += n,
            Err(e) => eprintln!("findr: {}: {e}", entry.path().display()),
        }
    }
    Ok(count)
}

fn search_file(re: &Regex, path: &Path, out: &mut impl WriteColor) -> Result<usize> {
    let bytes = std::fs::read(path)?;
    if is_binary(&bytes) {
        return Ok(0);
    }
    let text = String::from_utf8_lossy(&bytes);
    let mut count = 0;
    for (i, line) in text.lines().enumerate() {
        let Some(first) = re.find(line) else { continue };
        count += 1;
        out.set_color(ColorSpec::new().set_fg(Some(Color::Magenta)))?;
        write!(out, "{}", path.display())?;
        out.reset()?;
        write!(out, ":")?;
        out.set_color(ColorSpec::new().set_fg(Some(Color::Green)))?;
        write!(
            out,
            "{}:{}",
            i + 1,
            line[..first.start()].chars().count() + 1
        )?;
        out.reset()?;
        write!(out, ": ")?;
        let mut last = 0;
        for m in re.find_iter(line) {
            write!(out, "{}", &line[last..m.start()])?;
            out.set_color(ColorSpec::new().set_fg(Some(Color::Red)).set_bold(true))?;
            write!(out, "{}", m.as_str())?;
            out.reset()?;
            last = m.end();
        }
        writeln!(out, "{}", &line[last..])?;
    }
    Ok(count)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn detects_binary() {
        assert!(is_binary(b"ab\0cd"));
        assert!(!is_binary(b"plain text"));
    }
}
