use clap::{App, ArgMatches, SubCommand as ClapSubCommand};
use failure::Fallible;

use crate::cmd::{ArgMatchResult, CommandInvocation};

#[derive(Debug)]
struct Invocation {}

define_subcommand! {
    fn decorate_app<'a>(&self, app: App<'a, 'a>) -> App<'a, 'a> {
        app.subcommand(ClapSubCommand::with_name("pending").about("lists pending tasks"))
    }

    fn arg_match<'a>(&self, matches: &ArgMatches<'a>) -> ArgMatchResult {
        match matches.subcommand() {
            ("pending", _) => ArgMatchResult::Ok(Box::new(Invocation {})),
            // default to this command when no subcommand is given
            ("", _) => ArgMatchResult::Ok(Box::new(Invocation {})),
            _ => ArgMatchResult::None,
        }
    }
}

subcommand_invocation! {
    fn run(&self, command: &CommandInvocation) -> Fallible<()> {
        let working_set = command.get_replica().working_set().unwrap();
        for i in 1..working_set.len() {
            if let Some(ref task) = working_set[i] {
                println!("{}: {} - {:?}", i, task.get_uuid(), task);
            }
        }
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn parse_command() {
        with_subcommand_invocation!(vec!["task", "pending"], |_inv| {});
    }

    #[test]
    fn parse_command_default() {
        with_subcommand_invocation!(vec!["task"], |_inv| {});
    }
}
