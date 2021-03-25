use super::args::*;
use super::{ArgList, Subcommand};
use anyhow::bail;
use nom::{combinator::*, sequence::*, Err, IResult};

/// A command is the overall command that the CLI should execute.
///
/// It consists of some information common to all commands and a `Subcommand` identifying the
/// particular kind of behavior desired.
#[derive(Debug, PartialEq)]
pub(crate) struct Command {
    pub(crate) command_name: String,
    pub(crate) subcommand: Subcommand,
}

impl Command {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, Command> {
        fn to_command(input: (&str, Subcommand)) -> Result<Command, ()> {
            let command = Command {
                command_name: input.0.to_owned(),
                subcommand: input.1,
            };
            Ok(command)
        }
        map_res(
            all_consuming(tuple((arg_matching(any), Subcommand::parse))),
            to_command,
        )(input)
    }

    /// Parse a command from the given list of strings.
    pub fn from_argv(argv: &[&str]) -> anyhow::Result<Command> {
        match Command::parse(argv) {
            Ok((&[], cmd)) => Ok(cmd),
            Ok((trailing, _)) => bail!(
                "command line has trailing arguments: {:?}",
                trailing
            ),
            Err(Err::Incomplete(_)) => unreachable!(),
            Err(Err::Error(e)) => bail!("command line not recognized: {:?}", e),
            Err(Err::Failure(e)) => bail!("command line not recognized: {:?}", e),
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;

    // NOTE: most testing of specific subcommands is handled in `subcommand.rs`.

    #[test]
    fn test_version() {
        assert_eq!(
            Command::from_argv(argv!["task", "version"]).unwrap(),
            Command {
                subcommand: Subcommand::Version,
                command_name: s!("task"),
            }
        );
    }
}
