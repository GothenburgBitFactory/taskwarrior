use clap::{App, Arg, ArgMatches, SubCommand as ClapSubCommand};
use failure::{format_err, Fallible};
use taskchampion::Status;

use crate::cmd::{ArgMatchResult, CommandInvocation};

#[derive(Debug)]
struct Invocation {
    description: String,
}

define_subcommand! {
    fn decorate_app<'a>(&self, app: App<'a, 'a>) -> App<'a, 'a> {
        app.subcommand(
            ClapSubCommand::with_name("add").about("adds a task").arg(
                Arg::with_name("description")
                    .help("task description")
                    .required(true),
            ),
        )
    }

    fn arg_match<'a>(&self, matches: &ArgMatches<'a>) -> ArgMatchResult {
        match matches.subcommand() {
            ("add", Some(matches)) => {
                // TODO: .unwrap() would be safe here as description is required above
                let description: String = match matches.value_of("description") {
                    Some(v) => v.into(),
                    None => return ArgMatchResult::Err(format_err!("no description provided")),
                };
                ArgMatchResult::Ok(Box::new(Invocation { description }))
            }
            _ => ArgMatchResult::None,
        }
    }
}

subcommand_invocation! {
    fn run(&self, command: &CommandInvocation) -> Fallible<()> {
        let t = command
            .get_replica()?
            .new_task(Status::Pending, self.description.clone())
            .unwrap();
        println!("added task {}", t.get_uuid());
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn parse_command() {
        with_subcommand_invocation!(vec!["task", "add", "foo bar"], |inv: &Invocation| {
            assert_eq!(inv.description, "foo bar".to_string());
        });
    }
}
