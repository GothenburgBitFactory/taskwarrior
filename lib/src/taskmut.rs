use crate::{status::TCStatus, string::TCString, uuid::TCUuid};
use taskchampion::{TaskMut, Replica};
use std::cell::RefMut;

/// A mutable task.
///
/// A mutable task carries an exclusive reference to the replica,
/// meaning that no other replica operations can be carried out while
/// the mutable task still exists.
pub struct TCTaskMut {
    inner: TaskMut<'static>,
    replica: RefMut<Replica>,
}

impl TCTaskMut {
    /// Borrow a TCTaskMut from C as an argument.
    ///
    /// # Safety
    ///
    /// The pointer must not be NULL.  It is the caller's responsibility to ensure that the
    /// lifetime assigned to the reference and the lifetime of the TCTaskMut itself do not outlive
    /// the lifetime promised by C.
    pub(crate) unsafe fn from_arg_ref<'a>(tctask: *mut TCTaskMut) -> &'a mut Self {
        debug_assert!(!tctask.is_null());
        &mut *tctask
    }

    /// Convert this to a return value for handing off to C.
    pub(crate) fn return_val(self) -> *mut TCTaskMut {
        Box::into_raw(Box::new(self))
    }

    /// Create a new TCTaskMut, given a RefMut to the replica.
    pub(crate) fn from_immut(task: Task, rep: RefMut<Replica>) -> Self {
        // SAFETY:
        // This ref will be embedded in the TaskMut, and we will hang onto
        // the RefMut for that duration, guaranteeing no other mutable borrows.
        let rep_ref: &'static mut = unsafe { rep.deref_mut() }
        let task_mut = task.into_mut(rep_ref);
        TCTaskMut {
            inner: task_mut,
            replica: rep,
        }
    }
}

impl From<TaskMut> for TCTaskMut {
    fn from(rep: Replica) -> TCReplica {
        TCReplica {
            inner: RefCell::new(rep),
            error: None,
        }
    }
}


/// Free a task.  The given task must not be NULL.  The task must not be used after this function
/// returns, and must not be freed more than once.
#[no_mangle]
pub extern "C" fn tc_task_mut_free(task: *mut TCTaskMut) {
    debug_assert!(!task.is_null());
    // SAFETY:
    //  - task is not NULL (promised by caller)
    //  - task's lifetime exceeds the drop (promised by caller)
    //  - task does not outlive this function (promised by caller)
    drop(unsafe { Box::from_raw(task) });
}
