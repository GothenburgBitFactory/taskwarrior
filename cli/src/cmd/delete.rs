use clap::{App, ArgMatches, SubCommand as ClapSubCommand};
use failure::Fallible;
use taskchampion::Status;

use crate::cmd::{shared, ArgMatchResult, CommandInvocation};

#[derive(Debug)]
struct Invocation {
    task: String,
}

define_subcommand! {
    fn decorate_app<'a>(&self, app: App<'a, 'a>) -> App<'a, 'a> {
        app.subcommand(
            ClapSubCommand::with_name("delete")
                .about("mark the given task as deleted")
                .arg(shared::task_arg()))
    }

    fn arg_match<'a>(&self, matches: &ArgMatches<'a>) -> ArgMatchResult {
        match matches.subcommand() {
            ("delete", Some(matches)) => ArgMatchResult::Ok(Box::new(Invocation {
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
        let mut task = task.into_mut(&mut replica);
        task.stop()?;
        task.set_status(Status::Deleted)?;
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn parse_command() {
        with_subcommand_invocation!(vec!["task", "delete", "1"], |inv: &Invocation| {
            assert_eq!(inv.task, "1".to_string());
        });
    }
}

