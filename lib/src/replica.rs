use crate::traits::*;
use crate::util::err_to_tcstring;
use crate::{result::TCResult, status::TCStatus, string::TCString, task::TCTask, uuid::TCUuid};
use taskchampion::{Replica, StorageConfig};

/// A replica represents an instance of a user's task data, providing an easy interface
/// for querying and modifying that data.
///
/// # Error Handling
///
/// When a `tc_replica_..` function that returns a TCResult returns TC_RESULT_ERROR, then
/// `tc_replica_error` will return the error message.
///
/// # Safety
///
/// The `*TCReplica` returned from `tc_replica_new…` functions is owned by the caller and
/// must later be freed to avoid a memory leak.
///
/// Any function taking a `*TCReplica` requires:
///  - the pointer must not be NUL;
///  - the pointer must be one previously returned from a tc_… function;
///  - the memory referenced by the pointer must never be modified by C code; and
///  - except for `tc_replica_free`, ownership of a `*TCReplica` remains with the caller.
///
/// Once passed to `tc_replica_free`, a `*TCReplica` becmes invalid and must not be used again.
///
/// TCReplicas are not threadsafe.
pub struct TCReplica {
    /// The wrapped Replica
    inner: Replica,

    /// If true, this replica has an outstanding &mut (for a TaskMut)
    mut_borrowed: bool,

    /// The error from the most recent operation, if any
    error: Option<TCString<'static>>,
}

impl PassByPointer for TCReplica {}

impl TCReplica {
    /// Mutably borrow the inner Replica
    pub(crate) fn borrow_mut(&mut self) -> &mut Replica {
        if self.mut_borrowed {
            panic!("replica is already borrowed");
        }
        self.mut_borrowed = true;
        &mut self.inner
    }

    /// Release the borrow made by [`borrow_mut`]
    pub(crate) fn release_borrow(&mut self) {
        if !self.mut_borrowed {
            panic!("replica is not borrowed");
        }
        self.mut_borrowed = false;
    }
}

impl From<Replica> for TCReplica {
    fn from(rep: Replica) -> TCReplica {
        TCReplica {
            inner: rep,
            mut_borrowed: false,
            error: None,
        }
    }
}

/// Utility function to allow using `?` notation to return an error value.  This makes
/// a mutable borrow, because most Replica methods require a `&mut`.
fn wrap<'a, T, F>(rep: *mut TCReplica, f: F, err_value: T) -> T
where
    F: FnOnce(&mut Replica) -> anyhow::Result<T>,
{
    // SAFETY: see type docstring
    let rep: &'a mut TCReplica = unsafe { TCReplica::from_arg_ref_mut(rep) };
    if rep.mut_borrowed {
        panic!("replica is borrowed and cannot be used");
    }
    rep.error = None;
    match f(&mut rep.inner) {
        Ok(v) => v,
        Err(e) => {
            rep.error = Some(err_to_tcstring(e));
            err_value
        }
    }
}

/// Create a new TCReplica with an in-memory database.  The contents of the database will be
/// lost when it is freed.
#[no_mangle]
pub extern "C" fn tc_replica_new_in_memory() -> *mut TCReplica {
    let storage = StorageConfig::InMemory
        .into_storage()
        .expect("in-memory always succeeds");
    // SAFETY: see type docstring
    unsafe { TCReplica::from(Replica::new(storage)).return_val() }
}

/// Create a new TCReplica with an on-disk database having the given filename. On error, a string
/// is written to the `error_out` parameter (if it is not NULL) and NULL is returned.
#[no_mangle]
pub extern "C" fn tc_replica_new_on_disk<'a>(
    path: *mut TCString,
    error_out: *mut *mut TCString,
) -> *mut TCReplica {
    // SAFETY: see TCString docstring
    let path = unsafe { TCString::take_from_arg(path) };
    let storage_res = StorageConfig::OnDisk {
        taskdb_dir: path.to_path_buf(),
    }
    .into_storage();

    let storage = match storage_res {
        Ok(storage) => storage,
        Err(e) => {
            if !error_out.is_null() {
                unsafe {
                    *error_out = err_to_tcstring(e).return_val();
                }
            }
            return std::ptr::null_mut();
        }
    };

    // SAFETY: see type docstring
    unsafe { TCReplica::from(Replica::new(storage)).return_val() }
}

// TODO: tc_replica_all_tasks
// TODO: tc_replica_all_task_uuids
// TODO: tc_replica_working_set

/// Get an existing task by its UUID.
///
/// Returns NULL when the task does not exist, and on error.  Consult tc_replica_error
/// to distinguish the two conditions.
#[no_mangle]
pub extern "C" fn tc_replica_get_task(rep: *mut TCReplica, tcuuid: TCUuid) -> *mut TCTask {
    wrap(
        rep,
        |rep| {
            // SAFETY: see TCUuid docstring
            let uuid = unsafe { TCUuid::from_arg(tcuuid) };
            if let Some(task) = rep.get_task(uuid)? {
                Ok(TCTask::from(task).return_val())
            } else {
                Ok(std::ptr::null_mut())
            }
        },
        std::ptr::null_mut(),
    )
}

/// Create a new task.  The task must not already exist.
///
/// Returns the task, or NULL on error.
#[no_mangle]
pub extern "C" fn tc_replica_new_task(
    rep: *mut TCReplica,
    status: TCStatus,
    description: *mut TCString,
) -> *mut TCTask {
    // SAFETY: see TCString docstring
    let description = unsafe { TCString::take_from_arg(description) };
    wrap(
        rep,
        |rep| {
            let task = rep.new_task(status.into(), description.as_str()?.to_string())?;
            Ok(TCTask::from(task).return_val())
        },
        std::ptr::null_mut(),
    )
}

/// Create a new task.  The task must not already exist.
///
/// Returns the task, or NULL on error.
#[no_mangle]
pub extern "C" fn tc_replica_import_task_with_uuid(
    rep: *mut TCReplica,
    tcuuid: TCUuid,
) -> *mut TCTask {
    wrap(
        rep,
        |rep| {
            // SAFETY: see TCUuid docstring
            let uuid = unsafe { TCUuid::from_arg(tcuuid) };
            let task = rep.import_task_with_uuid(uuid)?;
            Ok(TCTask::from(task).return_val())
        },
        std::ptr::null_mut(),
    )
}

// TODO: tc_replica_sync

/// Undo local operations until the most recent UndoPoint.
///
/// If undone_out is not NULL, then on success it is set to 1 if operations were undone, or 0 if
/// there are no operations that can be done.
#[no_mangle]
pub extern "C" fn tc_replica_undo<'a>(rep: *mut TCReplica, undone_out: *mut i32) -> TCResult {
    wrap(
        rep,
        |rep| {
            let undone = if rep.undo()? { 1 } else { 0 };
            if !undone_out.is_null() {
                // SAFETY:
                //  - undone_out is not NULL (just checked)
                //  - undone_out is properly aligned (implicitly promised by caller)
                unsafe { *undone_out = undone };
            }
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

/// Get the latest error for a replica, or NULL if the last operation succeeded.  Subsequent calls
/// to this function will return NULL.  The rep pointer must not be NULL.  The caller must free the
/// returned string.
#[no_mangle]
pub extern "C" fn tc_replica_error<'a>(rep: *mut TCReplica) -> *mut TCString<'static> {
    // SAFETY: see type docstring
    let rep: &'a mut TCReplica = unsafe { TCReplica::from_arg_ref_mut(rep) };
    if let Some(tcstring) = rep.error.take() {
        // SAFETY: see TCString docstring
        unsafe { tcstring.return_val() }
    } else {
        std::ptr::null_mut()
    }
}

/// Free a replica.  The replica may not be used after this function returns and must not be freed
/// more than once.
#[no_mangle]
pub extern "C" fn tc_replica_free(rep: *mut TCReplica) {
    // SAFETY: see type docstring
    let replica = unsafe { TCReplica::take_from_arg(rep) };
    if replica.mut_borrowed {
        panic!("replica is borrowed and cannot be freed");
    }
    drop(replica);
}

// TODO: tc_replica_rebuild_working_set
// TODO: tc_replica_add_undo_point
