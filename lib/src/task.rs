use crate::{status::TCStatus, string::TCString, uuid::TCUuid};
use taskchampion::Task;

/// A task, as publicly exposed by this library.
///
/// A task carries no reference to the replica that created it, and can
/// be used until it is freed or converted to a TaskMut.
pub struct TCTask {
    inner: Task,
}

impl TCTask {
    pub(crate) fn as_ptr(task: Task) -> *mut TCTask {
        Box::into_raw(Box::new(TCTask { inner: task }))
    }
}

/// Utility function to allow using `?` notation to return an error value.
fn wrap<'a, T, F>(task: *const TCTask, f: F, err_value: T) -> T
where
    F: FnOnce(&Task) -> anyhow::Result<T>,
{
    let task: &'a Task = task_ref(task);
    match f(task) {
        Ok(v) => v,
        Err(e) => {
            /*
            let error = e.to_string();
            let error = match CString::new(error.as_bytes()) {
                Ok(e) => e,
                Err(_) => CString::new("(invalid error message)".as_bytes()).unwrap(),
            };
            */
            //task.error = Some(error);
            err_value
        }
    }
}

/// Utility function to safely convert *const TCTask into &Task
fn task_ref(task: *const TCTask) -> &'static Task {
    debug_assert!(!task.is_null());
    unsafe { &(*task).inner }
}

/// Get a task's UUID.
#[no_mangle]
pub extern "C" fn tc_task_get_uuid<'a>(task: *const TCTask) -> TCUuid {
    let task: &'a Task = task_ref(task);
    let uuid = task.get_uuid();
    uuid.into()
}

/// Get a task's status.
#[no_mangle]
pub extern "C" fn tc_task_get_status<'a>(task: *const TCTask) -> TCStatus {
    let task: &'a Task = task_ref(task);
    task.get_status().into()
}

/* TODO
 * into_mut
 * get_taskmap
 */

/// Get a task's description, or NULL if the task cannot be represented as a C string (e.g., if it
/// contains embedded NUL characters).
#[no_mangle]
pub extern "C" fn tc_task_get_description<'a>(task: *const TCTask) -> *mut TCString {
    wrap(
        task,
        |task| Ok(TCString::return_string(task.get_description())?),
        std::ptr::null_mut(),
    )
}

/* TODO
 * get_wait
 * is_waiting
 * is_active
 * has_tag
 * get_tags
 * get_annotations
 * get_uda
 * get_udas
 * get_legacy_uda
 * get_legacy_udas
 * get_modified
 */

/// Free a task.
#[no_mangle]
pub extern "C" fn tc_task_free<'a>(task: *mut TCTask) {
    debug_assert!(!task.is_null());
    drop(unsafe { Box::from_raw(task) });
}
