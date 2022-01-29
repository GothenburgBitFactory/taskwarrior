use crate::{replica::TCReplica, status::TCStatus, string::TCString, uuid::TCUuid};
use std::ops::Deref;
use taskchampion::{Task, TaskMut};

/// A task, as publicly exposed by this library.
///
/// A task begins in "immutable" mode.  It must be converted to "mutable" mode
/// to make any changes, and doing so requires exclusive access to the replica
/// until the task is freed or converted back to immutable mode.
///
/// A task carries no reference to the replica that created it, and can
/// be used until it is freed or converted to a TaskMut.
pub enum TCTask {
    Immutable(Task),
    Mutable(TaskMut<'static>),

    /// A transitional state for a TCTask as it goes from mutable to immutable.
    Invalid,
}

impl TCTask {
    /// Borrow a TCTask from C as an argument.
    ///
    /// # Safety
    ///
    /// The pointer must not be NULL.  It is the caller's responsibility to ensure that the
    /// lifetime assigned to the reference and the lifetime of the TCTask itself do not outlive
    /// the lifetime promised by C.
    pub(crate) unsafe fn from_arg_ref<'a>(tctask: *const TCTask) -> &'a Self {
        debug_assert!(!tctask.is_null());
        &*tctask
    }

    /// Borrow a TCTask from C as an argument, allowing mutation.
    ///
    /// # Safety
    ///
    /// The pointer must not be NULL.  It is the caller's responsibility to ensure that the
    /// lifetime assigned to the reference and the lifetime of the TCTask itself do not outlive
    /// the lifetime promised by C.
    pub(crate) unsafe fn from_arg_ref_mut<'a>(tctask: *mut TCTask) -> &'a mut Self {
        debug_assert!(!tctask.is_null());
        &mut *tctask
    }

    // TODO: from_arg_owned, use in drop

    /// Convert a TCTask to a return value for handing off to C.
    pub(crate) fn return_val(self) -> *mut TCTask {
        Box::into_raw(Box::new(self))
    }

    /// Make an immutable TCTask into a mutable TCTask.  Does nothing if the task
    /// is already mutable.
    fn to_mut(&mut self, tcreplica: &'static mut TCReplica) {
        *self = match std::mem::replace(self, TCTask::Invalid) {
            TCTask::Immutable(task) => {
                let rep_ref = tcreplica.borrow_mut();
                TCTask::Mutable(task.into_mut(rep_ref))
            }
            TCTask::Mutable(task) => TCTask::Mutable(task),
            TCTask::Invalid => unreachable!(),
        }
    }

    /// Make an mutable TCTask into a immutable TCTask.  Does nothing if the task
    /// is already immutable.
    fn to_immut(&mut self, tcreplica: &mut TCReplica) {
        *self = match std::mem::replace(self, TCTask::Invalid) {
            TCTask::Immutable(task) => TCTask::Immutable(task),
            TCTask::Mutable(task) => {
                tcreplica.release_borrow();
                TCTask::Immutable(task.into_immut())
            }
            TCTask::Invalid => unreachable!(),
        }
    }
}

impl From<Task> for TCTask {
    fn from(task: Task) -> TCTask {
        TCTask::Immutable(task)
    }
}

/// Utility function to get a shared reference to the underlying Task.
fn wrap<'a, T, F>(task: *const TCTask, f: F) -> T
where
    F: FnOnce(&Task) -> T,
{
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives 'a (promised by caller)
    let tctask: &'a TCTask = unsafe { TCTask::from_arg_ref(task) };
    let task: &'a Task = match tctask {
        TCTask::Immutable(t) => t,
        TCTask::Mutable(t) => t.deref(),
        TCTask::Invalid => unreachable!(),
    };
    f(task)
}

/// Utility function to get a mutable reference to the underlying Task.  The
/// TCTask must be mutable.
fn wrap_mut<'a, T, F>(task: *mut TCTask, f: F) -> T
where
    F: FnOnce(&mut TaskMut) -> anyhow::Result<T>,
{
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives 'a (promised by caller)
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref_mut(task) };
    let task: &'a mut TaskMut = match tctask {
        TCTask::Immutable(_) => panic!("Task is immutable"),
        TCTask::Mutable(ref mut t) => t,
        TCTask::Invalid => unreachable!(),
    };
    // TODO: add TCTask error handling, like replica
    f(task).unwrap()
}

/// Convert an immutable task into a mutable task.
///
/// The task is modified in-place, and becomes mutable.
///
/// The replica _cannot be used at all_ until this task is made immutable again.  This implies that
/// it is not allowed for more than one task associated with a replica to be mutable at any time.
///
/// Typical mutation of tasks is bracketed with `tc_task_to_mut` and `tc_task_to_immut`:
///
/// ```c
/// tc_task_to_mut(task, rep);
/// success = tc_task_done(task);
/// tc_task_to_immut(task, rep);
/// if (!success) { ... }
/// ```
#[no_mangle]
pub extern "C" fn tc_task_to_mut<'a>(task: *mut TCTask, rep: *mut TCReplica) {
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref_mut(task) };
    let tcreplica: &'static mut TCReplica = unsafe { TCReplica::from_arg_ref(rep) };
    tctask.to_mut(tcreplica);
}

/// Convert a mutable task into an immutable task.
///
/// The task is modified in-place, and becomes immutable.
///
/// The replica may be used freely after this call.
#[no_mangle]
pub extern "C" fn tc_task_to_immut<'a>(task: *mut TCTask, rep: *mut TCReplica) {
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref_mut(task) };
    let tcreplica: &'static mut TCReplica = unsafe { TCReplica::from_arg_ref(rep) };
    tctask.to_immut(tcreplica);
}

/// Get a task's UUID.
#[no_mangle]
pub extern "C" fn tc_task_get_uuid(task: *const TCTask) -> TCUuid {
    wrap(task, |task| task.get_uuid().into())
}

/// Get a task's status.
#[no_mangle]
pub extern "C" fn tc_task_get_status<'a>(task: *const TCTask) -> TCStatus {
    wrap(task, |task| task.get_status().into())
}

// TODO: get_taskmap

/// Get a task's description, or NULL if the task cannot be represented as a C string (e.g., if it
/// contains embedded NUL characters).
#[no_mangle]
pub extern "C" fn tc_task_get_description<'a>(task: *const TCTask) -> *mut TCString<'static> {
    wrap(task, |task| {
        let descr: TCString = task.get_description().into();
        descr.return_val()
    })
}

// TODO: :get_entry
// TODO: :get_wait
// TODO: :get_modified
// TODO: :is_waiting
// TODO: :is_active
// TODO: :has_tag
// TODO: :get_tags
// TODO: :get_annotations
// TODO: :get_uda
// TODO: :get_udas
// TODO: :get_legacy_uda
// TODO: :get_legacy_udas
// TODO: :get_modified

/// Set a mutable task's status.
///
/// Returns false on error.
#[no_mangle]
pub extern "C" fn tc_task_set_status<'a>(task: *mut TCTask, status: TCStatus) -> bool {
    wrap_mut(task, |task| {
        task.set_status(status.into())?;
        Ok(true)
    })
}

/// Set a mutable task's description.
///
/// Returns false on error.
#[no_mangle]
pub extern "C" fn tc_task_set_description<'a>(
    task: *mut TCTask,
    description: *mut TCString,
) -> bool {
    // SAFETY:
    //  - tcstring is not NULL (promised by caller)
    //  - caller is exclusive owner of tcstring (implicitly promised by caller)
    let description = unsafe { TCString::from_arg(description) };
    wrap_mut(task, |task| {
        task.set_description(description.as_str()?.to_string())?;
        Ok(true)
    })
}

// TODO: tc_task_set_description
// TODO: tc_task_set_entry
// TODO: tc_task_set_wait
// TODO: tc_task_set_modified
// TODO: tc_task_start
// TODO: tc_task_stop
// TODO: tc_task_done
// TODO: tc_task_delete
// TODO: tc_task_add_tag
// TODO: tc_task_remove_tag
// TODO: tc_task_add_annotation
// TODO: tc_task_remove_annotation
// TODO: tc_task_set_uda
// TODO: tc_task_remove_uda
// TODO: tc_task_set_legacy_uda
// TODO: tc_task_remove_legacy_uda

/// Free a task.  The given task must not be NULL and must be immutable.  The task must not be used
/// after this function returns, and must not be freed more than once.
///
/// The restriction that the task must be immutable may be lifted (TODO)
#[no_mangle]
pub extern "C" fn tc_task_free<'a>(task: *mut TCTask) {
    // convert the task to immutable before freeing
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref_mut(task) };
    if !matches!(tctask, TCTask::Immutable(_)) {
        // this limit is in place because we require the caller to supply a pointer
        // to the replica to make a task immutable, and no such pointer is available
        // here.
        panic!("Task must be immutable when freed");
    }
    drop(tctask);

    // SAFETY:
    //  - task is not NULL (promised by caller)
    //  - task's lifetime exceeds the drop (promised by caller)
    //  - task does not outlive this function (promised by caller)
    drop(unsafe { Box::from_raw(task) });
}
