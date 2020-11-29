use clap::{App, ArgMatches, SubCommand as ClapSubCommand};
use failure::Fallible;

use crate::cmd::{shared, ArgMatchResult, CommandInvocation};

#[derive(Debug)]
struct Invocation {
    task: String,
}

define_subcommand! {
    fn decorate_app<'a>(&self, app: App<'a, 'a>) -> App<'a, 'a> {
        app.subcommand(
            ClapSubCommand::with_name("stop")
                .about("stop the given task")
                .arg(shared::task_arg()))
    }

    fn arg_match<'a>(&self, matches: &ArgMatches<'a>) -> ArgMatchResult {
        match matches.subcommand() {
            ("stop", Some(matches)) => ArgMatchResult::Ok(Box::new(Invocation {
                task: matches.value_of("task").unwrap().into(),
            })),
            _ => ArgMatchResult::None,
        }
    }
}

subcommand_invocation! {
    fn run(&self, command: &CommandInvocation) -> Fallible<()> {
        let mut replica = command.get_replica()?;
        let task = shared::get_task(&mut replica, &self.task)?;
        task.into_mut(&mut replica).stop()?;
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn parse_command() {
        with_subcommand_invocation!(vec!["task", "stop", "1"], |inv: &Invocation| {
            assert_eq!(inv.task, "1".to_string());
        });
    }
}
