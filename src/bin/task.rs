extern crate clap;
use clap::{App, Arg, SubCommand};
use ot::{Replica, DB};
use uuid::Uuid;

fn main() {
    let matches = App::new("Rask")
        .version("0.1")
        .author("Dustin J. Mitchell <dustin@v.igoro.us>")
        .about("Replacement for TaskWarrior")
        .subcommand(
            SubCommand::with_name("add")
                .about("adds a task")
                .arg(Arg::with_name("title").help("task title").required(true)),
        )
        .subcommand(SubCommand::with_name("list").about("lists tasks"))
        .get_matches();

    let mut replica = Replica::new(DB::new().into());

    match matches.subcommand() {
        ("add", Some(matches)) => {
            let uuid = Uuid::new_v4();
            replica.create_task(uuid).unwrap();
            replica
                .update_task(uuid, "title", Some(matches.value_of("title").unwrap()))
                .unwrap();
        }
        ("list", _) => {
            for task in replica.all_tasks() {
                println!("{:?}", task);
            }
        }
        ("", None) => {
            unreachable!();
        }
        _ => unreachable!(),
    };
}
