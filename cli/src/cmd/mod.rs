use clap::{App, ArgMatches};
use failure::{Error, Fallible};

#[macro_use]
mod macros;
mod shared;

mod add;
mod gc;
mod info;
mod list;
mod pending;
mod sync;

/// Get a list of all subcommands in this crate
pub(crate) fn subcommands() -> Vec<Box<dyn SubCommand>> {
    vec![
        add::cmd(),
        gc::cmd(),
        list::cmd(),
        pending::cmd(),
        info::cmd(),
        sync::cmd(),
    ]
}

/// The result of a [`crate::cmd::SubCommand::arg_match`] call
pub(crate) enum ArgMatchResult {
    /// No match
    None,

    /// A good match
    Ok(Box<dyn SubCommandInvocation>),

    /// A match, but an issue with the command line
    Err(Error),
}

/// A subcommand represents a defined subcommand, and is typically a singleton.
pub(crate) trait SubCommand {
    /// Decorate the given [`clap::App`] appropriately for this subcommand
    fn decorate_app<'a>(&self, app: App<'a, 'a>) -> App<'a, 'a>;

    /// If this ArgMatches is for this command, return an appropriate invocation.
    fn arg_match<'a>(&self, matches: &ArgMatches<'a>) -> ArgMatchResult;
}

/// A subcommand invocation is specialized to a subcommand
pub(crate) trait SubCommandInvocation: std::fmt::Debug {
    fn run(&self, command: &CommandInvocation) -> Fallible<()>;

    // tests use downcasting, which requires a function to cast to Any
    #[cfg(test)]
    fn as_any(&self) -> &dyn std::any::Any;
}

pub use shared::CommandInvocation;
