use clap::{Args, Parser, Subcommand, ValueEnum};
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(
    name = "findr",
    version,
    about = "Find regex matches in directory names, file names or file contents, and replace text"
)]
pub struct Cli {
    #[command(subcommand)]
    pub command: Command,
}

#[derive(Subcommand, Debug)]
pub enum Command {
    /// Match directory names
    Dirs(SearchArgs),
    /// Match file names
    Files(SearchArgs),
    /// Match text inside files
    Text(SearchArgs),
    /// Replace text inside files (dry run unless --write is given)
    Replace(ReplaceArgs),
}

#[derive(Args, Debug, Clone)]
pub struct CommonArgs {
    /// Paths to search (defaults to the current directory)
    #[arg(default_value = ".")]
    pub paths: Vec<PathBuf>,
    /// Case-insensitive matching
    #[arg(short = 'i', long)]
    pub ignore_case: bool,
    /// Treat the pattern as a literal string
    #[arg(short = 'F', long)]
    pub fixed_strings: bool,
    /// Include/exclude paths by glob (prefix with ! to exclude); repeatable
    #[arg(short = 'g', long = "glob")]
    pub globs: Vec<String>,
    /// Search hidden files and directories
    #[arg(long)]
    pub hidden: bool,
    /// Do not respect .gitignore files
    #[arg(long)]
    pub no_ignore: bool,
    /// Maximum directory depth
    #[arg(short = 'd', long)]
    pub max_depth: Option<usize>,
    /// When to use color
    #[arg(long, value_enum, default_value_t = ColorWhen::Auto)]
    pub color: ColorWhen,
}

#[derive(Args, Debug)]
pub struct SearchArgs {
    /// Regular expression to search for
    pub pattern: String,
    #[command(flatten)]
    pub common: CommonArgs,
}

#[derive(Args, Debug)]
pub struct ReplaceArgs {
    /// Regular expression to search for
    pub pattern: String,
    /// Replacement text (supports $1 and ${name})
    pub replacement: String,
    /// Apply changes instead of previewing them
    #[arg(long)]
    pub write: bool,
    #[command(flatten)]
    pub common: CommonArgs,
}

#[derive(ValueEnum, Debug, Clone, Copy, PartialEq, Eq)]
pub enum ColorWhen {
    Auto,
    Always,
    Never,
}
