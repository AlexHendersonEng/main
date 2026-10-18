//! `rudatcom` command line entry point.

use clap::Parser;

/// Rust rewrite of the USAF Digital DATCOM.
#[derive(Parser)]
#[command(name = "rudatcom", version)]
struct Cli {}

fn main() {
    let _cli = Cli::parse();
    println!("rudatcom {}", rudatcom_core::version());
}
