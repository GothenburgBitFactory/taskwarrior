use crate::cmd::shared;
use clap::{App, Arg, ArgMatches, SubCommand as ClapSubCommand};
use failure::Fallible;

use crate::cmd::{ArgMatchResult, CommandInvocation};

#[derive(Debug)]
struct Invocation {
    task: String,
    description: String,
}

define_subcommand! {
    fn decorate_app<'a>(&self, app: App<'a, 'a>) -> App<'a, 'a> {
        app.subcommand(
            ClapSubCommand::with_name("modify").about("modifies a task")
                .arg(shared::task_arg())
            .arg(
                Arg::with_name("description")
                    .help("task description")
                    .required(true),
            ),
        )
    }

    fn arg_match<'a>(&self, matches: &ArgMatches<'a>) -> ArgMatchResult {
        match matches.subcommand() {
            ("modify", Some(matches)) => ArgMatchResult::Ok(Box::new(Invocation {
                task: matches.value_of("task").unwrap().into(),
                description: matches.value_of("description").unwrap().into(),
            })),
            _ => ArgMatchResult::None,
        }
    }
}

subcommand_invocation! {
    fn run(&self, command: &CommandInvocation) -> Fallible<()> {
        let mut replica = command.get_replica();
        let task = shared::get_task(&mut replica, &self.task)?;

        let mut task = task.into_mut(&mut replica);
        task.set_description(self.description.clone())?;
        println!("modified task {}", task.get_uuid());
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn parse_command() {
        with_subcommand_invocation!(
            vec!["task", "modify", "2", "foo bar"],
            |inv: &Invocation| {
                assert_eq!(inv.task, "2".to_string());
                assert_eq!(inv.description, "foo bar".to_string());
            }
        );
    }
}
