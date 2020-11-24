use clap::Arg;
use failure::{format_err, Fallible};
use taskchampion::{Replica, Task, Uuid};

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
