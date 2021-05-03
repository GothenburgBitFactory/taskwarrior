#![deny(clippy::all)]
#![allow(clippy::unnecessary_wraps)] // for Rust 1.50, https://github.com/rust-lang/rust-clippy/pull/6765
/*!
This crate implements the command-line interface to TaskChampion.

## Design

The crate is split into two parts: argument parsing (`argparse`) and command invocation (`invocation`).
Both are fairly complex operations, and the split serves both to isolate that complexity and to facilitate testing.

### Argparse

The TaskChampion command line API is modeled on TaskWarrior's API, which is far from that of a typical UNIX command.
Tools like `clap` and `structopt` are not flexible enough to handle this syntax.

Instead, the `argparse` module uses [nom](https://crates.io/crates/nom) to parse command lines as a sequence of words.
These parsers act on a list of strings, `&[&str]`, and at the top level return a `crate::argparse::Command`.
This is a wholly-owned repesentation of the command line's meaning, but with some interpretation.
For example, `task start`, `task stop`, and `task append` all map to a `crate::argparse::Subcommand::Modify` variant.

### Invocation

The `invocation` module executes a `Command`, given some settings and other ancillary data.
Most of its functionality is in common functions to handle filtering tasks, modifying tasks, and so on.

## Rust API

Note that this crate does not expose a Rust API for use from other crates.
For the public TaskChampion Rust API, see the `taskchampion` crate.

*/

use std::os::unix::ffi::OsStringExt;
use std::string::FromUtf8Error;

// NOTE: it's important that this 'mod' comes first so that the macros can be used in other modules
mod macros;

mod argparse;
mod invocation;
mod settings;
mod table;
mod usage;

use settings::Settings;

/// The main entry point for the command-line interface.  This builds an Invocation
/// from the particulars of the operating-system interface, and then executes it.
pub fn main() -> anyhow::Result<()> {
    env_logger::init();

    // parse the command line into a vector of &str, failing if
    // there are invalid utf-8 sequences.
    let argv: Vec<String> = std::env::args_os()
        .map(|oss| String::from_utf8(oss.into_vec()))
        .collect::<Result<_, FromUtf8Error>>()?;
    let argv: Vec<&str> = argv.iter().map(|s| s.as_ref()).collect();

    // parse the command line
    let command = argparse::Command::from_argv(&argv[..])?;

    // load the application settings
    let settings = Settings::read()?;

    invocation::invoke(command, settings)?;
    Ok(())
}
