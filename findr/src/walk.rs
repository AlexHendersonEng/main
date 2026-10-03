use crate::cli::CommonArgs;
use anyhow::{Context, Result};
use ignore::overrides::OverrideBuilder;
use ignore::{DirEntry, WalkBuilder};
use regex::{Regex, RegexBuilder};

pub fn build_regex(pattern: &str, common: &CommonArgs) -> Result<Regex> {
    let pat = if common.fixed_strings { regex::escape(pattern) } else { pattern.to_string() };
    RegexBuilder::new(&pat)
        .case_insensitive(common.ignore_case)
        .build()
        .with_context(|| format!("invalid pattern: {pattern}"))
}

/// Walks all configured paths, yielding entries (excluding the root paths' own traversal errors).
pub fn walk(common: &CommonArgs) -> Result<impl Iterator<Item = Result<DirEntry>>> {
    let mut builder = WalkBuilder::new(&common.paths[0]);
    for p in &common.paths[1..] {
        builder.add(p);
    }
    builder.hidden(!common.hidden).max_depth(common.max_depth);
    if common.no_ignore {
        builder.ignore(false).git_ignore(false).git_global(false).git_exclude(false).parents(false);
    }
    if !common.globs.is_empty() {
        let mut ov = OverrideBuilder::new(".");
        for g in &common.globs {
            ov.add(g).with_context(|| format!("invalid glob: {g}"))?;
        }
        builder.overrides(ov.build()?);
    }
    Ok(builder.build().map(|r| r.map_err(Into::into)))
}
