extern crate clap;
use clap::{App, Arg, SubCommand};
use std::path::Path;
use taskwarrior_rust::{taskstorage, Replica, Status, DB};
use uuid::Uuid;

fn main() {
    let matches = App::new("Rask")
        .version("0.1")
        .author("Dustin J. Mitchell <dustin@v.igoro.us>")
        .about("Replacement for TaskWarrior")
        .subcommand(
            SubCommand::with_name("add").about("adds a task").arg(
                Arg::with_name("descrpition")
                    .help("task descrpition")
                    .required(true),
            ),
        )
        .subcommand(SubCommand::with_name("list").about("lists tasks"))
        .get_matches();

    let mut replica = Replica::new(
        DB::new(Box::new(
            taskstorage::KVStorage::new(Path::new("/tmp/tasks")).unwrap(),
        ))
        .into(),
    );

    match matches.subcommand() {
        ("add", Some(matches)) => {
            let uuid = Uuid::new_v4();
            replica
                .new_task(
                    uuid,
                    Status::Pending,
                    matches.value_of("descrpition").unwrap().into(),
                )
                .unwrap();
        }
        ("list", _) => {
            for task in replica.all_tasks().unwrap() {
                println!("{:?}", task);
            }
        }
        ("", None) => {
            unreachable!();
        }
        _ => unreachable!(),
    };
}
