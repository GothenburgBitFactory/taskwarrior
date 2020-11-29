use crate::table;
use clap::{App, ArgMatches, SubCommand as ClapSubCommand};
use failure::Fallible;
use prettytable::{cell, row, Table};

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
        let mut replica = command.get_replica();
        let mut t = Table::new();
        t.set_format(table::format());
        t.set_titles(row![b->"id", b->"act", b->"description"]);
        for (uuid, task) in replica.all_tasks().unwrap() {
            let mut id = uuid.to_string();
            if let Some(i) = replica.get_working_set_index(&uuid)? {
                id = i.to_string();
            }
            let active = match task.is_active() {
                true => "*",
                false => "",
            };
            t.add_row(row![id, active, task.get_description()]);
        }
        t.printstd();
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
