use crate::cli::ColorWhen;
use std::io::IsTerminal;
use termcolor::ColorChoice;

pub fn color_choice(when: ColorWhen) -> ColorChoice {
    match when {
        ColorWhen::Always => ColorChoice::Always,
        ColorWhen::Never => ColorChoice::Never,
        ColorWhen::Auto if std::io::stdout().is_terminal() => ColorChoice::Auto,
        ColorWhen::Auto => ColorChoice::Never,
    }
}
