use clap::Arg;
use failure::{format_err, Fallible};
use std::env;
use std::ffi::OsString;
use taskchampion::{server, taskstorage, Replica, Task, Uuid};

pub(super) fn task_arg<'a>() -> Arg<'a, 'a> {
    Arg::with_name("task")
        .help("task id or uuid")
        .required(true)
}

pub(super) fn get_task<S: AsRef<str>>(replica: &mut Replica, task_arg: S) -> Fallible<Task> {
    let task_arg = task_arg.as_ref();

    // first try treating task as a working-set reference
    if let Ok(i) = task_arg.parse::<usize>() {
        if let Some(task) = replica.get_working_set_task(i)? {
            return Ok(task);
        }
    }

    if let Ok(uuid) = Uuid::parse_str(task_arg) {
        if let Some(task) = replica.get_task(&uuid)? {
            return Ok(task);
        }
    }

    Err(format_err!("Cannot interpret {:?} as a task", task_arg))
}

/// A command invocation contains all of the necessary regarding a single invocation of the CLI.
#[derive(Debug)]
pub struct CommandInvocation {
    pub(crate) subcommand: Box<dyn super::SubCommandInvocation>,
}

impl CommandInvocation {
    pub(crate) fn new(subcommand: Box<dyn super::SubCommandInvocation>) -> Self {
        Self { subcommand }
    }

    pub fn run(self) -> Fallible<()> {
        self.subcommand.run(&self)
    }

    // -- utilities for command invocations

    pub(super) fn get_replica(&self) -> Replica {
        // temporarily use $TASK_DB to locate the taskdb
        let taskdb_dir = env::var_os("TASK_DB").unwrap_or_else(|| OsString::from("/tmp/tasks"));
        Replica::new(Box::new(taskstorage::KVStorage::new(taskdb_dir).unwrap()))
    }

    pub(super) fn get_server(&self) -> Fallible<impl server::Server> {
        // temporarily use $SYNC_SERVER_ORIGIN for the sync server
        let sync_server_origin = env::var_os("SYNC_SERVER_ORIGIN")
            .map(|osstr| osstr.into_string().unwrap())
            .unwrap_or_else(|| String::from("http://localhost:8080"));
        Ok(server::RemoteServer::new(
            sync_server_origin,
            Uuid::parse_str("d5b55cbd-9a82-4860-9a39-41b67893b22f").unwrap(),
        ))
    }
}
