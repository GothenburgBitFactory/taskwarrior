use clap::{App, ArgMatches, SubCommand as ClapSubCommand};
use failure::Fallible;

use crate::cmd::{ArgMatchResult, CommandInvocation};

#[derive(Debug)]
struct Invocation {}

define_subcommand! {
    fn decorate_app<'a>(&self, app: App<'a, 'a>) -> App<'a, 'a> {
        app.subcommand(ClapSubCommand::with_name("list").about("lists tasks"))
    }

    fn arg_match<'a>(&self, matches: &ArgMatches<'a>) -> ArgMatchResult {
        match matches.subcommand() {
            ("list", _) => ArgMatchResult::Ok(Box::new(Invocation {})),
            _ => ArgMatchResult::None,
        }
    }
}

subcommand_invocation! {
    fn run(&self, command: &CommandInvocation) -> Fallible<()> {
        for (uuid, task) in command.get_replica().all_tasks().unwrap() {
            println!("{} - {:?}", uuid, task);
        }
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn parse_command() {
        with_subcommand_invocation!(vec!["task", "list"], |_inv| {});
    }
}
