use crate::traits::*;
use crate::types::*;
use crate::util::err_to_ruststring;
use std::convert::TryFrom;
use std::ops::Deref;
use std::ptr::NonNull;
use std::str::FromStr;
use taskchampion::chrono::{TimeZone, Utc};
use taskchampion::{Annotation, Tag, Task, TaskMut, Uuid};

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
/// # Safety
///
/// A task is an owned object, and must be freed with tc_task_free (or, if part of a list,
/// with tc_task_list_free).
///
/// Any function taking a `*TCTask` requires:
///  - the pointer must not be NUL;
///  - the pointer must be one previously returned from a tc_… function;
///  - the memory referenced by the pointer must never be modified by C code; and
///  - except for `tc_{task,task_list}_free`, ownership of a `*TCTask` remains with the caller.
///
/// Once passed to tc_task_free, a `*TCTask` becomes  invalid and must not be used again.
///
/// TCTasks are not threadsafe.
pub struct TCTask {
    /// The wrapped Task or TaskMut
    inner: Inner,

    /// The error from the most recent operation, if any
    error: Option<RustString<'static>>,
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

impl PassByPointer for TCTask {}

impl TCTask {
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
                let tcreplica_ref: &mut TCReplica =
                    unsafe { TCReplica::from_ptr_arg_ref_mut(tcreplica) };
                let rep_ref = tcreplica_ref.borrow_mut();
                Inner::Mutable(task.into_mut(rep_ref), tcreplica)
            }
            Inner::Mutable(task, tcreplica) => Inner::Mutable(task, tcreplica),
            Inner::Invalid => unreachable!(),
        }
    }

    /// Make an mutable TCTask into a immutable TCTask.  Does nothing if the task
    /// is already immutable.
    #[allow(clippy::wrong_self_convention)] // to_immut_mut is not better!
    fn to_immut(&mut self) {
        self.inner = match std::mem::replace(&mut self.inner, Inner::Invalid) {
            Inner::Immutable(task) => Inner::Immutable(task),
            Inner::Mutable(task, tcreplica) => {
                // SAFETY:
                //  - tcreplica is not null (promised by caller of to_mut, which created this
                //    variant)
                //  - tcreplica is still alive (promised by caller of to_mut)
                let tcreplica_ref: &mut TCReplica =
                    unsafe { TCReplica::from_ptr_arg_ref_mut(tcreplica) };
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
fn wrap<T, F>(task: *mut TCTask, f: F) -> T
where
    F: FnOnce(&Task) -> T,
{
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives this function (promised by caller)
    let tctask: &mut TCTask = unsafe { TCTask::from_ptr_arg_ref_mut(task) };
    let task: &Task = match &tctask.inner {
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
fn wrap_mut<T, F>(task: *mut TCTask, f: F, err_value: T) -> T
where
    F: FnOnce(&mut TaskMut) -> anyhow::Result<T>,
{
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives this function (promised by caller)
    let tctask: &mut TCTask = unsafe { TCTask::from_ptr_arg_ref_mut(task) };
    let task: &mut TaskMut = match tctask.inner {
        Inner::Immutable(_) => panic!("Task is immutable"),
        Inner::Mutable(ref mut t, _) => t,
        Inner::Invalid => unreachable!(),
    };
    tctask.error = None;
    match f(task) {
        Ok(rv) => rv,
        Err(e) => {
            tctask.error = Some(err_to_ruststring(e));
            err_value
        }
    }
}

impl TryFrom<RustString<'static>> for Tag {
    type Error = anyhow::Error;

    fn try_from(mut rstring: RustString) -> Result<Tag, anyhow::Error> {
        let tagstr = rstring.as_str()?;
        Tag::from_str(tagstr)
    }
}

/// TCTaskList represents a list of tasks.
///
/// The content of this struct must be treated as read-only: no fields or anything they reference
/// should be modified directly by C code.
///
/// When an item is taken from this list, its pointer in `items` is set to NULL.
#[repr(C)]
pub struct TCTaskList {
    /// number of tasks in items
    len: libc::size_t,

    /// total size of items (internal use only)
    _capacity: libc::size_t,

    /// array of pointers representing each task. these remain owned by the TCTaskList instance and
    /// will be freed by tc_task_list_free.  This pointer is never NULL for a valid TCTaskList,
    /// and the *TCTaskList at indexes 0..len-1 are not NULL.
    items: *mut Option<NonNull<TCTask>>,
}

impl CList for TCTaskList {
    type Element = Option<NonNull<TCTask>>;

    unsafe fn from_raw_parts(items: *mut Self::Element, len: usize, cap: usize) -> Self {
        TCTaskList {
            len,
            _capacity: cap,
            items,
        }
    }

    fn slice(&mut self) -> &mut [Self::Element] {
        // SAFETY:
        //  - because we have &mut self, we have read/write access to items[0..len]
        //  - all items are properly initialized Element's
        //  - return value lifetime is equal to &mmut self's, so access is exclusive
        //  - items and len came from Vec, so total size is < isize::MAX
        unsafe { std::slice::from_raw_parts_mut(self.items, self.len) }
    }

    fn into_raw_parts(self) -> (*mut Self::Element, usize, usize) {
        (self.items, self.len, self._capacity)
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
pub unsafe extern "C" fn tc_task_to_mut(task: *mut TCTask, tcreplica: *mut TCReplica) {
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives 'a (promised by caller)
    let tctask: &mut TCTask = unsafe { TCTask::from_ptr_arg_ref_mut(task) };
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
pub unsafe extern "C" fn tc_task_to_immut(task: *mut TCTask) {
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives 'a (promised by caller)
    let tctask: &mut TCTask = unsafe { TCTask::from_ptr_arg_ref_mut(task) };
    tctask.to_immut();
}

/// Get a task's UUID.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_uuid(task: *mut TCTask) -> TCUuid {
    wrap(task, |task| {
        // SAFETY:
        // - value is not allocated and need not be freed
        unsafe { TCUuid::return_val(task.get_uuid()) }
    })
}

/// Get a task's status.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_status(task: *mut TCTask) -> TCStatus {
    wrap(task, |task| task.get_status().into())
}

/// Get the underlying key/value pairs for this task.  The returned TCKVList is
/// a "snapshot" of the task and will not be updated if the task is subsequently
/// modified.  It is the caller's responsibility to free the TCKVList.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_taskmap(task: *mut TCTask) -> TCKVList {
    wrap(task, |task| {
        let vec: Vec<TCKV> = task
            .get_taskmap()
            .iter()
            .map(|(k, v)| {
                let key = RustString::from(k.as_ref());
                let value = RustString::from(v.as_ref());
                TCKV::as_ctype((key, value))
            })
            .collect();
        // SAFETY:
        //  - caller will free this list
        unsafe { TCKVList::return_val(vec) }
    })
}

/// Get a task's description, or NULL if the task cannot be represented as a C string (e.g., if it
/// contains embedded NUL characters).
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_description(task: *mut TCTask) -> TCString {
    wrap(task, |task| {
        let descr = task.get_description();
        // SAFETY:
        //  - caller promises to free this string
        unsafe { TCString::return_val(descr.into()) }
    })
}

/// Get the entry timestamp for a task (when it was created), or 0 if not set.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_entry(task: *mut TCTask) -> libc::time_t {
    wrap(task, |task| libc::time_t::as_ctype(task.get_entry()))
}

/// Get the wait timestamp for a task, or 0 if not set.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_wait(task: *mut TCTask) -> libc::time_t {
    wrap(task, |task| libc::time_t::as_ctype(task.get_wait()))
}

/// Get the modified timestamp for a task, or 0 if not set.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_modified(task: *mut TCTask) -> libc::time_t {
    wrap(task, |task| libc::time_t::as_ctype(task.get_modified()))
}

/// Check if a task is waiting.
#[no_mangle]
pub unsafe extern "C" fn tc_task_is_waiting(task: *mut TCTask) -> bool {
    wrap(task, |task| task.is_waiting())
}

/// Check if a task is active (started and not stopped).
#[no_mangle]
pub unsafe extern "C" fn tc_task_is_active(task: *mut TCTask) -> bool {
    wrap(task, |task| task.is_active())
}

/// Check if a task has the given tag.  If the tag is invalid, this function will return false, as
/// that (invalid) tag is not present. No error will be reported via `tc_task_error`.
#[no_mangle]
pub unsafe extern "C" fn tc_task_has_tag(task: *mut TCTask, tag: TCString) -> bool {
    // SAFETY:
    //  - tag is valid (promised by caller)
    //  - caller will not use tag after this call (convention)
    let tcstring = unsafe { TCString::val_from_arg(tag) };
    wrap(task, |task| {
        if let Ok(tag) = Tag::try_from(tcstring) {
            task.has_tag(&tag)
        } else {
            false
        }
    })
}

/// Get the tags for the task.
///
/// The caller must free the returned TCStringList instance.  The TCStringList instance does not
/// reference the task and the two may be freed in any order.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_tags(task: *mut TCTask) -> TCStringList {
    wrap(task, |task| {
        let vec: Vec<TCString> = task
            .get_tags()
            .map(|t| {
                // SAFETY:
                //  - this TCString will be freed via tc_string_list_free.
                unsafe { TCString::return_val(t.as_ref().into()) }
            })
            .collect();
        // SAFETY:
        //  - caller will free the list
        unsafe { TCStringList::return_val(vec) }
    })
}

/// Get the annotations for the task.
///
/// The caller must free the returned TCAnnotationList instance.  The TCStringList instance does not
/// reference the task and the two may be freed in any order.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_annotations(task: *mut TCTask) -> TCAnnotationList {
    wrap(task, |task| {
        let vec: Vec<TCAnnotation> = task
            .get_annotations()
            .map(|a| {
                let description = RustString::from(a.description);
                TCAnnotation::as_ctype((a.entry, description))
            })
            .collect();
        // SAFETY:
        //  - caller will free the list
        unsafe { TCAnnotationList::return_val(vec) }
    })
}

/// Get the named UDA from the task.
///
/// Returns a TCString with NULL ptr field if the UDA does not exist.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_uda(
    task: *mut TCTask,
    ns: TCString,
    key: TCString,
) -> TCString {
    wrap(task, |task| {
        // SAFETY:
        //  - ns is valid (promised by caller)
        //  - caller will not use ns after this call (convention)
        if let Ok(ns) = unsafe { TCString::val_from_arg(ns) }.as_str() {
            // SAFETY: same
            if let Ok(key) = unsafe { TCString::val_from_arg(key) }.as_str() {
                if let Some(value) = task.get_uda(ns, key) {
                    // SAFETY:
                    // - caller will free this string (caller promises)
                    return unsafe { TCString::return_val(value.into()) };
                }
            }
        }
        TCString::default()
    })
}

/// Get the named legacy UDA from the task.
///
/// Returns NULL if the UDA does not exist.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_legacy_uda(task: *mut TCTask, key: TCString) -> TCString {
    wrap(task, |task| {
        // SAFETY:
        //  - key is valid (promised by caller)
        //  - caller will not use key after this call (convention)
        if let Ok(key) = unsafe { TCString::val_from_arg(key) }.as_str() {
            if let Some(value) = task.get_legacy_uda(key) {
                // SAFETY:
                // - caller will free this string (caller promises)
                return unsafe { TCString::return_val(value.into()) };
            }
        }
        TCString::default()
    })
}

/// Get all UDAs for this task.
///
/// Legacy UDAs are represented with an empty string in the ns field.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_udas(task: *mut TCTask) -> TCUdaList {
    wrap(task, |task| {
        let vec: Vec<TCUda> = task
            .get_udas()
            .map(|((ns, key), value)| {
                // SAFETY:
                //  - will be freed by tc_uda_list_free
                unsafe {
                    TCUda::return_val(Uda {
                        ns: Some(ns.into()),
                        key: key.into(),
                        value: value.into(),
                    })
                }
            })
            .collect();
        // SAFETY:
        //  - caller will free this list
        unsafe { TCUdaList::return_val(vec) }
    })
}

/// Get all UDAs for this task.
///
/// All TCUdas in this list have a NULL ns field.  The entire UDA key is
/// included in the key field.  The caller must free the returned list.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_legacy_udas(task: *mut TCTask) -> TCUdaList {
    wrap(task, |task| {
        let vec: Vec<TCUda> = task
            .get_legacy_udas()
            .map(|(key, value)| {
                // SAFETY:
                //  - will be freed by tc_uda_list_free
                unsafe {
                    TCUda::return_val(Uda {
                        ns: None,
                        key: key.into(),
                        value: value.into(),
                    })
                }
            })
            .collect();
        // SAFETY:
        //  - caller will free this list
        unsafe { TCUdaList::return_val(vec) }
    })
}

/// Set a mutable task's status.
#[no_mangle]
pub unsafe extern "C" fn tc_task_set_status(task: *mut TCTask, status: TCStatus) -> TCResult {
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
pub unsafe extern "C" fn tc_task_set_description(
    task: *mut TCTask,
    description: TCString,
) -> TCResult {
    // SAFETY:
    //  - description is valid (promised by caller)
    //  - caller will not use description after this call (convention)
    let mut description = unsafe { TCString::val_from_arg(description) };
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
pub unsafe extern "C" fn tc_task_set_entry(task: *mut TCTask, entry: libc::time_t) -> TCResult {
    wrap_mut(
        task,
        |task| {
            // SAFETY: any time_t value is a valid timestamp
            task.set_entry(unsafe { entry.from_ctype() })?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Set a mutable task's wait timestamp.  Pass wait=0 to unset the wait field.
#[no_mangle]
pub unsafe extern "C" fn tc_task_set_wait(task: *mut TCTask, wait: libc::time_t) -> TCResult {
    wrap_mut(
        task,
        |task| {
            // SAFETY: any time_t value is a valid timestamp
            task.set_wait(unsafe { wait.from_ctype() })?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Set a mutable task's modified timestamp.  The value cannot be zero.
#[no_mangle]
pub unsafe extern "C" fn tc_task_set_modified(
    task: *mut TCTask,
    modified: libc::time_t,
) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.set_modified(
                // SAFETY: any time_t value is a valid timestamp
                unsafe { modified.from_ctype() }
                    .ok_or_else(|| anyhow::anyhow!("modified cannot be zero"))?,
            )?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Start a task.
#[no_mangle]
pub unsafe extern "C" fn tc_task_start(task: *mut TCTask) -> TCResult {
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
pub unsafe extern "C" fn tc_task_stop(task: *mut TCTask) -> TCResult {
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
pub unsafe extern "C" fn tc_task_done(task: *mut TCTask) -> TCResult {
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
pub unsafe extern "C" fn tc_task_delete(task: *mut TCTask) -> TCResult {
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
pub unsafe extern "C" fn tc_task_add_tag(task: *mut TCTask, tag: TCString) -> TCResult {
    // SAFETY:
    //  - tag is valid (promised by caller)
    //  - caller will not use tag after this call (convention)
    let tcstring = unsafe { TCString::val_from_arg(tag) };
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

/// Remove a tag from a mutable task.
#[no_mangle]
pub unsafe extern "C" fn tc_task_remove_tag(task: *mut TCTask, tag: TCString) -> TCResult {
    // SAFETY:
    //  - tag is valid (promised by caller)
    //  - caller will not use tag after this call (convention)
    let tcstring = unsafe { TCString::val_from_arg(tag) };
    wrap_mut(
        task,
        |task| {
            let tag = Tag::try_from(tcstring)?;
            task.remove_tag(&tag)?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Add an annotation to a mutable task.  This call takes ownership of the
/// passed annotation, which must not be used after the call returns.
#[no_mangle]
pub unsafe extern "C" fn tc_task_add_annotation(
    task: *mut TCTask,
    annotation: *mut TCAnnotation,
) -> TCResult {
    // SAFETY:
    //  - annotation is not NULL (promised by caller)
    //  - annotation is return from a tc_string_.. so is valid
    //  - caller will not use annotation after this call
    let (entry, description) =
        unsafe { TCAnnotation::take_val_from_arg(annotation, TCAnnotation::default()) };
    wrap_mut(
        task,
        |task| {
            let description = description.into_string()?;
            task.add_annotation(Annotation { entry, description })?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Remove an annotation from a mutable task.
#[no_mangle]
pub unsafe extern "C" fn tc_task_remove_annotation(task: *mut TCTask, entry: i64) -> TCResult {
    wrap_mut(
        task,
        |task| {
            task.remove_annotation(Utc.timestamp(entry, 0))?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Set a UDA on a mutable task.
#[no_mangle]
pub unsafe extern "C" fn tc_task_set_uda(
    task: *mut TCTask,
    ns: TCString,
    key: TCString,
    value: TCString,
) -> TCResult {
    // safety:
    //  - ns is valid (promised by caller)
    //  - caller will not use ns after this call (convention)
    let mut ns = unsafe { TCString::val_from_arg(ns) };
    // SAFETY: same
    let mut key = unsafe { TCString::val_from_arg(key) };
    // SAFETY: same
    let mut value = unsafe { TCString::val_from_arg(value) };
    wrap_mut(
        task,
        |task| {
            task.set_uda(ns.as_str()?, key.as_str()?, value.as_str()?.to_string())?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Remove a UDA fraom a mutable task.
#[no_mangle]
pub unsafe extern "C" fn tc_task_remove_uda(
    task: *mut TCTask,
    ns: TCString,
    key: TCString,
) -> TCResult {
    // safety:
    //  - ns is valid (promised by caller)
    //  - caller will not use ns after this call (convention)
    let mut ns = unsafe { TCString::val_from_arg(ns) };
    // SAFETY: same
    let mut key = unsafe { TCString::val_from_arg(key) };
    wrap_mut(
        task,
        |task| {
            task.remove_uda(ns.as_str()?, key.as_str()?)?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Set a legacy UDA on a mutable task.
#[no_mangle]
pub unsafe extern "C" fn tc_task_set_legacy_uda(
    task: *mut TCTask,
    key: TCString,
    value: TCString,
) -> TCResult {
    // safety:
    //  - key is valid (promised by caller)
    //  - caller will not use key after this call (convention)
    let mut key = unsafe { TCString::val_from_arg(key) };
    // SAFETY: same
    let mut value = unsafe { TCString::val_from_arg(value) };
    wrap_mut(
        task,
        |task| {
            task.set_legacy_uda(key.as_str()?.to_string(), value.as_str()?.to_string())?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Remove a UDA fraom a mutable task.
#[no_mangle]
pub unsafe extern "C" fn tc_task_remove_legacy_uda(task: *mut TCTask, key: TCString) -> TCResult {
    // safety:
    //  - key is valid (promised by caller)
    //  - caller will not use key after this call (convention)
    let mut key = unsafe { TCString::val_from_arg(key) };
    wrap_mut(
        task,
        |task| {
            task.remove_legacy_uda(key.as_str()?.to_string())?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Get all dependencies for a task.
#[no_mangle]
pub unsafe extern "C" fn tc_task_get_dependencies(task: *mut TCTask) -> TCUuidList {
    wrap(task, |task| {
        let vec: Vec<TCUuid> = task
            .get_dependencies()
            .map(|u| {
                // SAFETY:
                //  - value is not allocated
                unsafe { TCUuid::return_val(u) }
            })
            .collect();
        // SAFETY:
        //  - caller will free this list
        unsafe { TCUuidList::return_val(vec) }
    })
}

/// Add a dependency.
#[no_mangle]
pub unsafe extern "C" fn tc_task_add_dependency(task: *mut TCTask, dep: TCUuid) -> TCResult {
    // SAFETY:
    //  - tcuuid is a valid TCUuid (all byte patterns are valid)
    let dep: Uuid = unsafe { TCUuid::val_from_arg(dep) };
    wrap_mut(
        task,
        |task| {
            task.add_dependency(dep)?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Remove a dependency.
#[no_mangle]
pub unsafe extern "C" fn tc_task_remove_dependency(task: *mut TCTask, dep: TCUuid) -> TCResult {
    // SAFETY:
    //  - tcuuid is a valid TCUuid (all byte patterns are valid)
    let dep: Uuid = unsafe { TCUuid::val_from_arg(dep) };
    wrap_mut(
        task,
        |task| {
            task.remove_dependency(dep)?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Get the latest error for a task, or a string NULL ptr field if the last operation succeeded.
/// Subsequent calls to this function will return NULL.  The task pointer must not be NULL.  The
/// caller must free the returned string.
#[no_mangle]
pub unsafe extern "C" fn tc_task_error(task: *mut TCTask) -> TCString {
    // SAFETY:
    //  - task is not null (promised by caller)
    //  - task outlives 'a (promised by caller)
    let task: &mut TCTask = unsafe { TCTask::from_ptr_arg_ref_mut(task) };
    if let Some(rstring) = task.error.take() {
        // SAFETY:
        //  - caller promises to free this value
        unsafe { TCString::return_val(rstring) }
    } else {
        TCString::default()
    }
}

/// Free a task.  The given task must not be NULL.  The task must not be used after this function
/// returns, and must not be freed more than once.
///
/// If the task is currently mutable, it will first be made immutable.
#[no_mangle]
pub unsafe extern "C" fn tc_task_free(task: *mut TCTask) {
    // SAFETY:
    //  - task is not NULL (promised by caller)
    //  - caller will not use the TCTask after this (promised by caller)
    let mut tctask = unsafe { TCTask::take_from_ptr_arg(task) };

    // convert to immut if it was mutable
    tctask.to_immut();

    drop(tctask);
}

/// Take an item from a TCTaskList.  After this call, the indexed item is no longer associated
/// with the list and becomes the caller's responsibility, just as if it had been returned from
/// `tc_replica_get_task`.
///
/// The corresponding element in the `items` array will be set to NULL.  If that field is already
/// NULL (that is, if the item has already been taken), this function will return NULL.  If the
/// index is out of bounds, this function will also return NULL.
///
/// The passed TCTaskList remains owned by the caller.
#[no_mangle]
pub unsafe extern "C" fn tc_task_list_take(tasks: *mut TCTaskList, index: usize) -> *mut TCTask {
    // SAFETY:
    //  - tasks is not NULL and points to a valid TCTaskList (caller is not allowed to
    //    modify the list directly, and tc_task_list_take leaves the list valid)
    let p = unsafe { take_optional_pointer_list_item(tasks, index) };
    if let Some(p) = p {
        p.as_ptr()
    } else {
        std::ptr::null_mut()
    }
}

/// Free a TCTaskList instance.  The instance, and all TCTaskList it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCTaskList.
#[no_mangle]
pub unsafe extern "C" fn tc_task_list_free(tasks: *mut TCTaskList) {
    // SAFETY:
    //  - tasks is not NULL and points to a valid TCTaskList (caller is not allowed to
    //    modify the list directly, and tc_task_list_take leaves the list valid)
    //  - caller promises not to use the value after return
    unsafe { drop_optional_pointer_list(tasks) };
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn empty_list_has_non_null_pointer() {
        let tasks = unsafe { TCTaskList::return_val(Vec::new()) };
        assert!(!tasks.items.is_null());
        assert_eq!(tasks.len, 0);
        assert_eq!(tasks._capacity, 0);
    }

    #[test]
    fn free_sets_null_pointer() {
        let mut tasks = unsafe { TCTaskList::return_val(Vec::new()) };
        // SAFETY: testing expected behavior
        unsafe { tc_task_list_free(&mut tasks) };
        assert!(tasks.items.is_null());
        assert_eq!(tasks.len, 0);
        assert_eq!(tasks._capacity, 0);
    }
}
