use clap::{App, AppSettings};
use failure::Fallible;
use std::ffi::OsString;

mod cmd;
pub(crate) mod settings;
mod table;

use cmd::ArgMatchResult;
pub(crate) use cmd::CommandInvocation;

/// Parse the given command line and return an as-yet un-executed CommandInvocation.
pub fn parse_command_line<I, T>(iter: I) -> Fallible<CommandInvocation>
where
    I: IntoIterator<Item = T>,
    T: Into<OsString> + Clone,
{
    let subcommands = cmd::subcommands();

    let mut app = App::new("TaskChampion")
        .version(env!("CARGO_PKG_VERSION"))
        .about("Personal task-tracking")
        .setting(AppSettings::ColoredHelp);

    for subcommand in subcommands.iter() {
        app = subcommand.decorate_app(app);
    }

    let matches = app.get_matches_from_safe(iter)?;

    for subcommand in subcommands.iter() {
        match subcommand.arg_match(&matches) {
            ArgMatchResult::Ok(invocation) => return Ok(CommandInvocation::new(invocation)),
            ArgMatchResult::Err(err) => return Err(err),
            ArgMatchResult::None => {}
        }
    }

    // one of the subcommands also matches the lack of subcommands, so this never
    // occurrs.
    unreachable!()
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_parse_command_line_success() -> Fallible<()> {
        // This just verifies that one of the subcommands works; the subcommands themselves
        // are tested in their own unit tests.
        parse_command_line(vec!["task", "pending"].iter())?;
        Ok(())
    }

    #[test]
    fn test_parse_command_line_failure() {
        assert!(parse_command_line(vec!["task", "--no-such-arg"].iter()).is_err());
    }
}
