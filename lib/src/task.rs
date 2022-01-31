use crate::{
    replica::TCReplica, result::TCResult, status::TCStatus, string::TCString, uuid::TCUuid,
};
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
///
/// All `tc_task_..` functions taking a task as an argument require that it not be NULL.
pub enum TCTask {
    /// A regular, immutable task
    Immutable(Task),

    /// A mutable task, together with the replica to which it holds an exclusive
    /// reference.
    Mutable(TaskMut<'static>, *mut TCReplica),

    /// A transitional state for a TCTask as it goes from mutable to immutable and back.  A task
    /// can only be in this state outside of [`to_mut`] and [`to_immut`] if a panic occurs during
    /// one of those methods.
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

    /// Take a TCTask from C as an argument.
    ///
    /// # Safety
    ///
    /// The pointer must not be NULL.  The pointer becomes invalid before this function returns.
    pub(crate) unsafe fn from_arg<'a>(tctask: *mut TCTask) -> Self {
        debug_assert!(!tctask.is_null());
        *Box::from_raw(tctask)
    }

    /// Convert a TCTask to a return value for handing off to C.
    pub(crate) fn return_val(self) -> *mut TCTask {
        Box::into_raw(Box::new(self))
    }

    /// Make an immutable TCTask into a mutable TCTask.  Does nothing if the task
    /// is already mutable.
    ///
    /// # Safety
    ///
    /// The tcreplica pointer must not be NULL, and the replica it points to must not
    /// be freed before TCTask.to_immut completes.
    unsafe fn to_mut(&mut self, tcreplica: *mut TCReplica) {
        *self = match std::mem::replace(self, TCTask::Invalid) {
            TCTask::Immutable(task) => {
                // SAFETY:
                //  - tcreplica is not null (promised by caller)
                //  - tcreplica outlives the pointer in this variant (promised by caller)
                let tcreplica_ref: &mut TCReplica = TCReplica::from_arg_ref(tcreplica);
                let rep_ref = tcreplica_ref.borrow_mut();
                TCTask::Mutable(task.into_mut(rep_ref), tcreplica)
            }
            TCTask::Mutable(task, tcreplica) => TCTask::Mutable(task, tcreplica),
            TCTask::Invalid => unreachable!(),
        }
    }

    /// Make an mutable TCTask into a immutable TCTask.  Does nothing if the task
    /// is already immutable.
    fn to_immut(&mut self) {
        *self = match std::mem::replace(self, TCTask::Invalid) {
            TCTask::Immutable(task) => TCTask::Immutable(task),
            TCTask::Mutable(task, tcreplica) => {
                // SAFETY:
                //  - tcreplica is not null (promised by caller of to_mut, which created this
                //    variant)
                //  - tcreplica is still alive (promised by caller of to_mut)
                let tcreplica_ref: &mut TCReplica = unsafe { TCReplica::from_arg_ref(tcreplica) };
                tcreplica_ref.release_borrow();
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

/// Utility function to get a shared reference to the underlying Task.  All Task getters
/// are error-free, so this does not handle errors.
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
        TCTask::Mutable(t, _) => t.deref(),
        TCTask::Invalid => unreachable!(),
    };
    f(task)
}

/// Utility function to get a mutable reference to the underlying Task.  The
/// TCTask must be mutable.  The inner function may use `?` syntax to return an
/// error, which will be represented with the `err_value` returned to C.
fn wrap_mut<'a, T, F>(task: *mut TCTask, f: F, err_value: T) -> T
where
    F: FnOnce(&mut TaskMut) -> anyhow::Result<T>,
{
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives 'a (promised by caller)
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref_mut(task) };
    let task: &'a mut TaskMut = match tctask {
        TCTask::Immutable(_) => panic!("Task is immutable"),
        TCTask::Mutable(ref mut t, _) => t,
        TCTask::Invalid => unreachable!(),
    };
    match f(task) {
        Ok(rv) => rv,
        Err(e) => {
            // TODO: add TCTask error handling, like replica
            err_value
        }
    }
}

/// Convert an immutable task into a mutable task.
///
/// The task must not be NULL. It is modified in-place, and becomes mutable.
///
/// The replica must not be NULL. After this function returns, the replica _cannot be used at all_
/// until this task is made immutable again.  This implies that it is not allowed for more than one
/// task associated with a replica to be mutable at any time.
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
pub extern "C" fn tc_task_to_mut<'a>(task: *mut TCTask, tcreplica: *mut TCReplica) {
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives 'a (promised by caller)
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref_mut(task) };
    // SAFETY:
    //  - tcreplica is not NULL (promised by caller)
    //  - tcreplica lives until later call to to_immut via tc_task_to_immut (promised by caller,
    //    who cannot call tc_replica_free during this time)
    unsafe { tctask.to_mut(tcreplica) };
}

/// Convert a mutable task into an immutable task.
///
/// The task must not be NULL.  It is modified in-place, and becomes immutable.
///
/// The replica passed to `tc_task_to_mut` may be used freely after this call.
#[no_mangle]
pub extern "C" fn tc_task_to_immut<'a>(task: *mut TCTask) {
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives 'a (promised by caller)
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref_mut(task) };
    tctask.to_immut();
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

// TODO: tc_task_get_taskmap (?? then we have to wrap a map..)

/// Get a task's description, or NULL if the task cannot be represented as a C string (e.g., if it
/// contains embedded NUL characters).
#[no_mangle]
pub extern "C" fn tc_task_get_description<'a>(task: *const TCTask) -> *mut TCString<'static> {
    wrap(task, |task| {
        let descr: TCString = task.get_description().into();
        descr.return_val()
    })
}

// TODO: tc_task_get_entry
// TODO: tc_task_get_wait
// TODO: tc_task_get_modified
// TODO: tc_task_is_waiting

/// Check if a task is active (started and not stopped).
#[no_mangle]
pub extern "C" fn tc_task_is_active<'a>(task: *const TCTask) -> bool {
    wrap(task, |task| task.is_active())
}

// TODO: tc_task_has_tag
// TODO: tc_task_get_tags
// TODO: tc_task_get_annotations
// TODO: tc_task_get_uda
// TODO: tc_task_get_udas
// TODO: tc_task_get_legacy_uda
// TODO: tc_task_get_legacy_udas
// TODO: tc_task_get_modified

/// Set a mutable task's status.
///
/// Returns TC_RESULT_TRUE on success and TC_RESULT_ERROR on failure.
#[no_mangle]
pub extern "C" fn tc_task_set_status<'a>(task: *mut TCTask, status: TCStatus) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.set_status(status.into())?;
            Ok(TCResult::True)
        },
        TCResult::Error,
    )
}

/// Set a mutable task's description.
///
/// Returns TC_RESULT_TRUE on success and TC_RESULT_ERROR on failure.
#[no_mangle]
pub extern "C" fn tc_task_set_description<'a>(
    task: *mut TCTask,
    description: *mut TCString,
) -> TCResult {
    // SAFETY:
    //  - tcstring is not NULL (promised by caller)
    //  - caller is exclusive owner of tcstring (implicitly promised by caller)
    let description = unsafe { TCString::from_arg(description) };
    wrap_mut(
        task,
        |task| {
            task.set_description(description.as_str()?.to_string())?;
            Ok(TCResult::True)
        },
        TCResult::Error,
    )
}

// TODO: tc_task_set_description
// TODO: tc_task_set_entry
// TODO: tc_task_set_wait
// TODO: tc_task_set_modified

/// Start a task.
///
/// Returns TC_RESULT_TRUE on success and TC_RESULT_ERROR on failure.
#[no_mangle]
pub extern "C" fn tc_task_start<'a>(task: *mut TCTask) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.start()?;
            Ok(TCResult::True)
        },
        TCResult::Error,
    )
}

/// Stop a task.
///
/// Returns TC_RESULT_TRUE on success and TC_RESULT_ERROR on failure.
#[no_mangle]
pub extern "C" fn tc_task_stop<'a>(task: *mut TCTask) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.stop()?;
            Ok(TCResult::True)
        },
        TCResult::Error,
    )
}

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

/// Free a task.  The given task must not be NULL.  The task must not be used after this function
/// returns, and must not be freed more than once.
///
/// If the task is currently mutable, it will first be made immutable.
#[no_mangle]
pub extern "C" fn tc_task_free<'a>(task: *mut TCTask) {
    // SAFETY:
    //  - rep is not NULL (promised by caller)
    //  - caller will not use the TCTask after this (promised by caller)
    let mut tctask = unsafe { TCTask::from_arg(task) };

    // convert to immut if it was mutable
    tctask.to_immut();

    drop(tctask);
}
