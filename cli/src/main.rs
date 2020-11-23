use clap::{App, Arg, SubCommand};
use std::path::Path;
use taskchampion::{taskstorage, Replica, Status};
use uuid::Uuid;

fn main() {
    let matches = App::new("Rask")
        .version("0.1")
        .author("Dustin J. Mitchell <dustin@v.igoro.us>")
        .about("Replacement for TaskWarrior")
        .subcommand(
            SubCommand::with_name("add").about("adds a task").arg(
                Arg::with_name("description")
                    .help("task description")
                    .required(true),
            ),
        )
        .subcommand(SubCommand::with_name("list").about("lists tasks"))
        .subcommand(SubCommand::with_name("pending").about("lists pending tasks"))
        .subcommand(SubCommand::with_name("gc").about("run garbage collection"))
        .get_matches();

    let mut replica = Replica::new(Box::new(
        taskstorage::KVStorage::new(Path::new("/tmp/tasks")).unwrap(),
    ));

    match matches.subcommand() {
        ("add", Some(matches)) => {
            let uuid = Uuid::new_v4();
            replica
                .new_task(
                    uuid,
                    Status::Pending,
                    matches.value_of("description").unwrap().into(),
                )
                .unwrap();
        }
        ("list", _) => {
            for (uuid, task) in replica.all_tasks().unwrap() {
                println!("{} - {:?}", uuid, task);
            }
        }
        ("pending", _) => {
            let working_set = replica.working_set().unwrap();
            for i in 1..working_set.len() {
                if let Some(ref task) = working_set[i] {
                    println!("{}: {} - {:?}", i, task.get_uuid(), task);
                }
            }
        }
        ("gc", _) => {
            replica.gc().unwrap();
        }
        ("", None) => {
            unreachable!();
        }
        _ => unreachable!(),
    };
}
