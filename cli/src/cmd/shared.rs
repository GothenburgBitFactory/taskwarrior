use clap::Arg;
use failure::{format_err, Fallible};
use taskchampion::{Replica, Task};
use uuid::Uuid;

pub(super) fn task_arg<'a>() -> Arg<'a, 'a> {
    Arg::with_name("task")
        .help("task id or uuid")
        .required(true)
}

pub(super) fn get_task<S: AsRef<str>>(replica: &mut Replica, task_arg: S) -> Fallible<Task> {
    let task_arg = task_arg.as_ref();

    // first try treating task as a working-set reference
    match task_arg.parse::<usize>() {
        Ok(i) => {
            if let Some(task) = replica.get_working_set_task(i)? {
                return Ok(task);
            }
        }
        Err(_) => {}
    }

    match Uuid::parse_str(task_arg) {
        Ok(uuid) => {
            if let Some(task) = replica.get_task(&uuid)? {
                return Ok(task);
            }
        }
        Err(_) => {}
    }

    Err(format_err!("Cannot interpret {:?} as a task", task_arg))
}
