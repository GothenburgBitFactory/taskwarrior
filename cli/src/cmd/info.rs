use crate::table;
use clap::{App, ArgMatches, SubCommand as ClapSubCommand};
use failure::Fallible;
use prettytable::{cell, row, Table};

use crate::cmd::{shared, ArgMatchResult, CommandInvocation};

#[derive(Debug)]
struct Invocation {
    task: String,
}

define_subcommand! {
    fn decorate_app<'a>(&self, app: App<'a, 'a>) -> App<'a, 'a> {
        app.subcommand(
            ClapSubCommand::with_name("info")
                .about("info on the given task")
                .arg(shared::task_arg()))
    }

    fn arg_match<'a>(&self, matches: &ArgMatches<'a>) -> ArgMatchResult {
        match matches.subcommand() {
            ("info", Some(matches)) => ArgMatchResult::Ok(Box::new(Invocation {
                task: matches.value_of("task").unwrap().into(),
            })),
            _ => ArgMatchResult::None,
        }
    }
}

subcommand_invocation! {
    fn run(&self, command: &CommandInvocation) -> Fallible<()> {
        let task = shared::get_task(&mut command.get_replica(), &self.task)?;

        let mut t = Table::new();
        t.set_format(table::format());
        t.add_row(row![b->"Uuid", task.get_uuid()]);
        t.add_row(row![b->"Description", task.get_description()]);
        t.add_row(row![b->"Status", task.get_status()]);
        t.printstd();
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn parse_command() {
        with_subcommand_invocation!(vec!["task", "info", "1"], |inv: &Invocation| {
            assert_eq!(inv.task, "1".to_string());
        });
    }
}
