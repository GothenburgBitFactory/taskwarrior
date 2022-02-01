use crate::util::err_to_tcstring;
use crate::{
    replica::TCReplica, result::TCResult, status::TCStatus, string::TCString, uuid::TCUuid,
};
use chrono::{DateTime, TimeZone, Utc};
use std::convert::TryFrom;
use std::ops::Deref;
use std::str::FromStr;
use taskchampion::{Tag, Task, TaskMut};

/// A task, as publicly exposed by this library.
///
/// A task begins in "immutable" mode.  It must be converted to "mutable" mode
/// to make any changes, and doing so requires exclusive access to the replica
/// until the task is freed or converted back to immutable mode.
///
/// An immutable task carries no reference to the replica that created it, and can be used until it
/// is freed or converted to a TaskMut.  A mutable task carries a reference to the replica and
/// must be freed or made immutable before the replica is freed.
///
/// All `tc_task_..` functions taking a task as an argument require that it not be NULL.
///
/// When a `tc_task_..` function that returns a TCResult returns TC_RESULT_ERROR, then
/// `tc_task_error` will return the error message.
///
/// TCTasks are not threadsafe.
pub struct TCTask {
    /// The wrapped Task or TaskMut
    inner: Inner,

    /// The error from the most recent operation, if any
    error: Option<TCString<'static>>,
}

enum Inner {
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
    pub(crate) unsafe fn from_arg_ref<'a>(tctask: *mut TCTask) -> &'a mut Self {
        debug_assert!(!tctask.is_null());
        // SAFETY: see docstring
        unsafe { &mut *tctask }
    }

    /// Take a TCTask from C as an argument.
    ///
    /// # Safety
    ///
    /// The pointer must not be NULL.  The pointer becomes invalid before this function returns.
    pub(crate) unsafe fn from_arg<'a>(tctask: *mut TCTask) -> Self {
        debug_assert!(!tctask.is_null());
        // SAFETY: see docstring
        unsafe { *Box::from_raw(tctask) }
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
        self.inner = match std::mem::replace(&mut self.inner, Inner::Invalid) {
            Inner::Immutable(task) => {
                // SAFETY:
                //  - tcreplica is not null (promised by caller)
                //  - tcreplica outlives the pointer in this variant (promised by caller)
                let tcreplica_ref: &mut TCReplica = unsafe { TCReplica::from_arg_ref(tcreplica) };
                let rep_ref = tcreplica_ref.borrow_mut();
                Inner::Mutable(task.into_mut(rep_ref), tcreplica)
            }
            Inner::Mutable(task, tcreplica) => Inner::Mutable(task, tcreplica),
            Inner::Invalid => unreachable!(),
        }
    }

    /// Make an mutable TCTask into a immutable TCTask.  Does nothing if the task
    /// is already immutable.
    fn to_immut(&mut self) {
        self.inner = match std::mem::replace(&mut self.inner, Inner::Invalid) {
            Inner::Immutable(task) => Inner::Immutable(task),
            Inner::Mutable(task, tcreplica) => {
                // SAFETY:
                //  - tcreplica is not null (promised by caller of to_mut, which created this
                //    variant)
                //  - tcreplica is still alive (promised by caller of to_mut)
                let tcreplica_ref: &mut TCReplica = unsafe { TCReplica::from_arg_ref(tcreplica) };
                tcreplica_ref.release_borrow();
                Inner::Immutable(task.into_immut())
            }
            Inner::Invalid => unreachable!(),
        }
    }
}

impl From<Task> for TCTask {
    fn from(task: Task) -> TCTask {
        TCTask {
            inner: Inner::Immutable(task),
            error: None,
        }
    }
}

/// Utility function to get a shared reference to the underlying Task.  All Task getters
/// are error-free, so this does not handle errors.
fn wrap<'a, T, F>(task: *mut TCTask, f: F) -> T
where
    F: FnOnce(&Task) -> T,
{
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives 'a (promised by caller)
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref(task) };
    let task: &'a Task = match &tctask.inner {
        Inner::Immutable(t) => t,
        Inner::Mutable(t, _) => t.deref(),
        Inner::Invalid => unreachable!(),
    };
    tctask.error = None;
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
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref(task) };
    let task: &'a mut TaskMut = match tctask.inner {
        Inner::Immutable(_) => panic!("Task is immutable"),
        Inner::Mutable(ref mut t, _) => t,
        Inner::Invalid => unreachable!(),
    };
    tctask.error = None;
    match f(task) {
        Ok(rv) => rv,
        Err(e) => {
            tctask.error = Some(err_to_tcstring(e));
            err_value
        }
    }
}

impl TryFrom<TCString<'_>> for Tag {
    type Error = anyhow::Error;

    fn try_from(tcstring: TCString) -> Result<Tag, anyhow::Error> {
        let tagstr = tcstring.as_str()?;
        Tag::from_str(tagstr)
    }
}

/// Convert a DateTime<Utc> to a libc::time_t, or zero if not set.
fn to_time_t(timestamp: Option<DateTime<Utc>>) -> libc::time_t {
    timestamp
        .map(|ts| ts.timestamp() as libc::time_t)
        .unwrap_or(0 as libc::time_t)
}

/// Convert a libc::time_t to Option<DateTime<Utc>>, treating time zero as None
fn to_datetime(time: libc::time_t) -> Option<DateTime<Utc>> {
    if time == 0 {
        None
    } else {
        Some(Utc.timestamp(time as i64, 0))
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
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref(task) };
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
    let tctask: &'a mut TCTask = unsafe { TCTask::from_arg_ref(task) };
    tctask.to_immut();
}

/// Get a task's UUID.
#[no_mangle]
pub extern "C" fn tc_task_get_uuid(task: *mut TCTask) -> TCUuid {
    wrap(task, |task| task.get_uuid().into())
}

/// Get a task's status.
#[no_mangle]
pub extern "C" fn tc_task_get_status<'a>(task: *mut TCTask) -> TCStatus {
    wrap(task, |task| task.get_status().into())
}

// TODO: tc_task_get_taskmap (?? then we have to wrap a map..)

/// Get a task's description, or NULL if the task cannot be represented as a C string (e.g., if it
/// contains embedded NUL characters).
#[no_mangle]
pub extern "C" fn tc_task_get_description<'a>(task: *mut TCTask) -> *mut TCString<'static> {
    wrap(task, |task| {
        let descr: TCString = task.get_description().into();
        descr.return_val()
    })
}

/// Get the entry timestamp for a task (when it was created), or 0 if not set.
#[no_mangle]
pub extern "C" fn tc_task_get_entry<'a>(task: *mut TCTask) -> libc::time_t {
    wrap(task, |task| to_time_t(task.get_entry()))
}

/// Get the wait timestamp for a task, or 0 if not set.
#[no_mangle]
pub extern "C" fn tc_task_get_wait<'a>(task: *mut TCTask) -> libc::time_t {
    wrap(task, |task| to_time_t(task.get_wait()))
}

/// Get the modified timestamp for a task, or 0 if not set.
#[no_mangle]
pub extern "C" fn tc_task_get_modified<'a>(task: *mut TCTask) -> libc::time_t {
    wrap(task, |task| to_time_t(task.get_modified()))
}

/// Check if a task is waiting.
#[no_mangle]
pub extern "C" fn tc_task_is_waiting(task: *mut TCTask) -> bool {
    wrap(task, |task| task.is_waiting())
}

/// Check if a task is active (started and not stopped).
#[no_mangle]
pub extern "C" fn tc_task_is_active(task: *mut TCTask) -> bool {
    wrap(task, |task| task.is_active())
}

/// Check if a task has the given tag.  If the tag is invalid, this function will simply return
/// false with no error from `tc_task_error`.  The given tag must not be NULL.
#[no_mangle]
pub extern "C" fn tc_task_has_tag<'a>(task: *mut TCTask, tag: *mut TCString) -> bool {
    // SAFETY:
    //  - tcstring is not NULL (promised by caller)
    //  - caller is exclusive owner of tcstring (implicitly promised by caller)
    let tcstring = unsafe { TCString::from_arg(tag) };
    wrap(task, |task| {
        if let Ok(tag) = Tag::try_from(tcstring) {
            task.has_tag(&tag)
        } else {
            false
        }
    })
}

// TODO: tc_task_get_tags
// TODO: tc_task_get_annotations
// TODO: tc_task_get_uda
// TODO: tc_task_get_udas
// TODO: tc_task_get_legacy_uda
// TODO: tc_task_get_legacy_udas

/// Set a mutable task's status.
#[no_mangle]
pub extern "C" fn tc_task_set_status<'a>(task: *mut TCTask, status: TCStatus) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.set_status(status.into())?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Set a mutable task's description.
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
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Set a mutable task's entry (creation time).  Pass entry=0 to unset
/// the entry field.
#[no_mangle]
pub extern "C" fn tc_task_set_entry(task: *mut TCTask, entry: libc::time_t) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.set_entry(to_datetime(entry))?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Set a mutable task's wait timestamp.  Pass wait=0 to unset the wait field.
#[no_mangle]
pub extern "C" fn tc_task_set_wait(task: *mut TCTask, wait: libc::time_t) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.set_wait(to_datetime(wait))?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Set a mutable task's modified timestamp.  The value cannot be zero.
#[no_mangle]
pub extern "C" fn tc_task_set_modified(task: *mut TCTask, modified: libc::time_t) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.set_modified(
                to_datetime(modified).ok_or_else(|| anyhow::anyhow!("modified cannot be zero"))?,
            )?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Start a task.
#[no_mangle]
pub extern "C" fn tc_task_start(task: *mut TCTask) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.start()?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Stop a task.
#[no_mangle]
pub extern "C" fn tc_task_stop(task: *mut TCTask) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.stop()?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Mark a task as done.
#[no_mangle]
pub extern "C" fn tc_task_done(task: *mut TCTask) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.done()?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Mark a task as deleted.
#[no_mangle]
pub extern "C" fn tc_task_delete(task: *mut TCTask) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.delete()?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Add a tag to a mutable task.
#[no_mangle]
pub extern "C" fn tc_task_add_tag(task: *mut TCTask, tag: *mut TCString) -> TCResult {
    // SAFETY:
    //  - tcstring is not NULL (promised by caller)
    //  - caller is exclusive owner of tcstring (implicitly promised by caller)
    let tcstring = unsafe { TCString::from_arg(tag) };
    wrap_mut(
        task,
        |task| {
            let tag = Tag::try_from(tcstring)?;
            task.add_tag(&tag)?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

// TODO: tc_task_remove_tag
// TODO: tc_task_add_annotation
// TODO: tc_task_remove_annotation
// TODO: tc_task_set_uda
// TODO: tc_task_remove_uda
// TODO: tc_task_set_legacy_uda
// TODO: tc_task_remove_legacy_uda

/// Get the latest error for a task, or NULL if the last operation succeeded.  Subsequent calls
/// to this function will return NULL.  The task pointer must not be NULL.  The caller must free the
/// returned string.
#[no_mangle]
pub extern "C" fn tc_task_error<'a>(task: *mut TCTask) -> *mut TCString<'static> {
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives 'a (promised by caller)
    let task: &'a mut TCTask = unsafe { TCTask::from_arg_ref(task) };
    if let Some(tcstring) = task.error.take() {
        tcstring.return_val()
    } else {
        std::ptr::null_mut()
    }
}

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
