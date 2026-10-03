use crate::cli::{CommonArgs, SearchArgs};
use crate::walk::{build_regex, walk};
use anyhow::Result;

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum Kind {
    Dir,
    File,
}

/// Prints entries of `kind` whose basename matches. Returns the match count.
pub fn run(args: &SearchArgs, kind: Kind) -> Result<usize> {
    let re = build_regex(&args.pattern, &args.common)?;
    search(&re, &args.common, kind, &mut std::io::stdout().lock())
}

pub fn search(
    re: &regex::Regex,
    common: &CommonArgs,
    kind: Kind,
    out: &mut impl std::io::Write,
) -> Result<usize> {
    let mut count = 0;
    for entry in walk(common)? {
        let entry = match entry {
            Ok(e) => e,
            Err(e) => {
                eprintln!("findr: {e}");
                continue;
            }
        };
        if entry.depth() == 0 && entry.path().as_os_str() == "." {
            continue;
        }
        let Some(ft) = entry.file_type() else { continue };
        let wanted = match kind {
            Kind::Dir => ft.is_dir(),
            Kind::File => ft.is_file(),
        };
        if !wanted {
            continue;
        }
        if re.is_match(&entry.file_name().to_string_lossy()) {
            writeln!(out, "{}", entry.path().display())?;
            count += 1;
        }
    }
    Ok(count)
}
