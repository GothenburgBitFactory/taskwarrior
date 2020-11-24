use clap::{Error as ClapError, ErrorKind};
use std::process::exit;
use taskchampion_cli::parse_command_line;

enum Output {
    Stdout,
    Stderr,
}
use Output::*;

fn bail<E: std::fmt::Display>(err: E, output: Output, code: i32) -> ! {
    match output {
        Stdout => println!("{}", err),
        Stderr => eprintln!("{}", err),
    }
    exit(code)
}

fn main() {
    let command = match parse_command_line(std::env::args_os()) {
        Ok(command) => command,
        Err(err) => {
            match err.downcast::<ClapError>() {
                Ok(err) => {
                    if err.kind == ErrorKind::HelpDisplayed
                        || err.kind == ErrorKind::VersionDisplayed
                    {
                        // --help and --version go to stdout and succeed
                        bail(err, Stdout, 0)
                    } else {
                        // other clap errors exit with failure
                        bail(err, Stderr, 1)
                    }
                }
                Err(err) => bail(err, Stderr, 1),
            }
        }
    };

    if let Err(err) = command.run() {
        bail(err, Stderr, 1)
    }
}
