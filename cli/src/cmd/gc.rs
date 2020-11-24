use crate::cmd::{ArgMatchResult, CommandInvocation};
use clap::{App, ArgMatches, SubCommand as ClapSubCommand};
use failure::Fallible;

#[derive(Debug)]
struct Invocation {}

define_subcommand! {
    fn decorate_app<'a>(&self, app: App<'a, 'a>) -> App<'a, 'a> {
        app.subcommand(ClapSubCommand::with_name("gc").about("run garbage collection"))
    }

    fn arg_match<'a>(&self, matches: &ArgMatches<'a>) -> ArgMatchResult {
        match matches.subcommand() {
            ("gc", _) => ArgMatchResult::Ok(Box::new(Invocation {})),
            _ => ArgMatchResult::None,
        }
    }
}

subcommand_invocation! {
    fn run(&self, command: &CommandInvocation) -> Fallible<()> {
        command.get_replica().gc()?;
        println!("garbage collected.");
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn parse_command() {
        with_subcommand_invocation!(vec!["task", "gc"], |_inv| {});
    }
}
