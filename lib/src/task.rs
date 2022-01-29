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

    /// Convert a TCTask to a return value for handing off to C.
    pub(crate) fn return_val(self) -> *mut TCTask {
        Box::into_raw(Box::new(self))
    }
}

impl From<Task> for TCTask {
    fn from(task: Task) -> TCTask {
        TCTask { inner: task }
    }
}

/// Get a task's UUID.
#[no_mangle]
pub extern "C" fn tc_task_get_uuid<'a>(task: *const TCTask) -> TCUuid {
    // SAFETY:
    //  - task is not NULL (promised by caller)
    //  - task's lifetime exceeds this function (promised by caller)
    let task: &'a Task = &unsafe { TCTask::from_arg_ref(task) }.inner;
    let uuid = task.get_uuid();
    uuid.into()
}

/// Get a task's status.
#[no_mangle]
pub extern "C" fn tc_task_get_status<'a>(task: *const TCTask) -> TCStatus {
    // SAFETY:
    //  - task is not NULL (promised by caller)
    //  - task's lifetime exceeds this function (promised by caller)
    let task: &'a Task = &unsafe { TCTask::from_arg_ref(task) }.inner;
    task.get_status().into()
}

// TODO: into_mut
// TODO: get_taskmap

/// Get a task's description, or NULL if the task cannot be represented as a C string (e.g., if it
/// contains embedded NUL characters).
#[no_mangle]
pub extern "C" fn tc_task_get_description<'a>(task: *const TCTask) -> *mut TCString<'static> {
    // SAFETY:
    //  - task is not NULL (promised by caller)
    //  - task's lifetime exceeds this function (promised by caller)
    let task: &'a Task = &unsafe { TCTask::from_arg_ref(task) }.inner;
    let descr: TCString = task.get_description().into();
    descr.return_val()
}

// TODO: :get_wait
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

/// Free a task.  The given task must not be NULL.  The task must not be used after this function
/// returns, and must not be freed more than once.
#[no_mangle]
pub extern "C" fn tc_task_free<'a>(task: *mut TCTask) {
    debug_assert!(!task.is_null());
    // SAFETY:
    //  - task is not NULL (promised by caller)
    //  - task's lifetime exceeds the drop (promised by caller)
    //  - task does not outlive this function (promised by caller)
    drop(unsafe { Box::from_raw(task) });
}
