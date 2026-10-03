mod cli;
mod names;
mod output;
mod replace;
mod text;
mod walk;

use clap::Parser;
use cli::{Cli, Command};
use std::process::ExitCode;

fn run(cli: Cli) -> anyhow::Result<usize> {
    match cli.command {
        Command::Dirs(a) => names::run(&a, names::Kind::Dir),
        Command::Files(a) => names::run(&a, names::Kind::File),
        Command::Text(a) => text::run(&a),
        Command::Replace(a) => replace::run(&a),
    }
}

fn main() -> ExitCode {
    match run(Cli::parse()) {
        Ok(0) => ExitCode::from(1),
        Ok(_) => ExitCode::SUCCESS,
        Err(e) => {
            eprintln!("findr: {e:#}");
            ExitCode::from(2)
        }
    }
}
