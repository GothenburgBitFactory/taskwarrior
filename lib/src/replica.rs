use crate::{result::TCResult, status::TCStatus, string::TCString, task::TCTask, uuid::TCUuid};
use taskchampion::{Replica, StorageConfig, Uuid};

/// A replica represents an instance of a user's task data, providing an easy interface
/// for querying and modifying that data.
///
/// TCReplicas are not threadsafe.
pub struct TCReplica {
    // TODO: make this a RefCell so that it can be take()n when holding a mut ref
    inner: Replica,
    error: Option<TCString<'static>>,
}

/// Utility function to safely convert *mut TCReplica into &mut TCReplica
fn rep_ref(rep: *mut TCReplica) -> &'static mut TCReplica {
    debug_assert!(!rep.is_null());
    unsafe { &mut *rep }
}

fn err_to_tcstring(e: impl std::string::ToString) -> TCString<'static> {
    TCString::from(e.to_string())
}

/// Utility function to allow using `?` notation to return an error value.
fn wrap<'a, T, F>(rep: *mut TCReplica, f: F, err_value: T) -> T
where
    F: FnOnce(&mut Replica) -> anyhow::Result<T>,
{
    let rep: &'a mut TCReplica = rep_ref(rep);
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
    Box::into_raw(Box::new(TCReplica {
        inner: Replica::new(storage),
        error: None,
    }))
}

/// Create a new TCReplica with an on-disk database.  On error, a string is written to the
/// `error_out` parameter (if it is not NULL) and NULL is returned.
#[no_mangle]
pub extern "C" fn tc_replica_new_on_disk<'a>(
    path: *mut TCString,
    error_out: *mut *mut TCString,
) -> *mut TCReplica {
    let path = TCString::from_arg(path);
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

    Box::into_raw(Box::new(TCReplica {
        inner: Replica::new(storage),
        error: None,
    }))
}

// TODO: tc_replica_all_tasks
// TODO: tc_replica_all_task_uuids
// TODO: tc_replica_working_set
// TODO: tc_replica_get_task

/// Create a new task.  The task must not already exist.
///
/// Returns the task, or NULL on error.
#[no_mangle]
pub extern "C" fn tc_replica_new_task(
    rep: *mut TCReplica,
    status: TCStatus,
    description: *mut TCString,
) -> *mut TCTask {
    wrap(
        rep,
        |rep| {
            let description = TCString::from_arg(description);
            let task = rep.new_task(status.into(), description.as_str()?.to_string())?;
            Ok(TCTask::as_ptr(task))
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
    uuid: TCUuid,
) -> *mut TCTask {
    wrap(
        rep,
        |rep| {
            let uuid: Uuid = uuid.into();
            let task = rep.import_task_with_uuid(uuid)?;
            Ok(TCTask::as_ptr(task))
        },
        std::ptr::null_mut(),
    )
}

// TODO: tc_replica_sync

/// Undo local operations until the most recent UndoPoint.
///
/// Returns TC_RESULT_TRUE if an undo occurred, TC_RESULT_FALSE if there are no operations
/// to be undone, or TC_RESULT_ERROR on error.
#[no_mangle]
pub extern "C" fn tc_replica_undo<'a>(rep: *mut TCReplica) -> TCResult {
    wrap(
        rep,
        |rep| {
            Ok(if rep.undo()? {
                TCResult::True
            } else {
                TCResult::False
            })
        },
        TCResult::Error,
    )
}

/// Get the latest error for a replica, or NULL if the last operation succeeded.
/// Subsequent calls to this function will return NULL.  The caller must free the
/// returned string.
#[no_mangle]
pub extern "C" fn tc_replica_error<'a>(rep: *mut TCReplica) -> *mut TCString<'static> {
    let rep: &'a mut TCReplica = rep_ref(rep);
    if let Some(tcstring) = rep.error.take() {
        tcstring.return_val()
    } else {
        std::ptr::null_mut()
    }
}

/// Free a TCReplica.
#[no_mangle]
pub extern "C" fn tc_replica_free(rep: *mut TCReplica) {
    debug_assert!(!rep.is_null());
    drop(unsafe { Box::from_raw(rep) });
}

/*
 * - tc_replica_rebuild_working_set
 * - tc_replica_add_undo_point
 */
