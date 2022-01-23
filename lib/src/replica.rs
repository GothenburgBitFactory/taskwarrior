use crate::{status::TCStatus, string::TCString, task::TCTask};
use libc::c_char;
use std::ffi::CString;
use taskchampion::{Replica, StorageConfig};

/// A replica represents an instance of a user's task data, providing an easy interface
/// for querying and modifying that data.
pub struct TCReplica {
    // TODO: make this a RefCell so that it can be take()n when holding a mut ref
    inner: Replica,
    error: Option<CString>,
}

/// Utility function to safely convert *mut TCReplica into &mut TCReplica
fn rep_ref(rep: *mut TCReplica) -> &'static mut TCReplica {
    debug_assert!(!rep.is_null());
    unsafe { &mut *rep }
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
            let error = e.to_string();
            let error = match CString::new(error.as_bytes()) {
                Ok(e) => e,
                Err(_) => CString::new("(invalid error message)".as_bytes()).unwrap(),
            };
            rep.error = Some(error);
            err_value
        }
    }
}

/// Create a new TCReplica.
///
/// If path is NULL, then an in-memory replica is created.  Otherwise, path is the path to the
/// on-disk storage for this replica.  The path argument is no longer referenced after return.
///
/// Returns NULL on error; see tc_replica_error.
///
/// TCReplicas are not threadsafe.
#[no_mangle]
pub extern "C" fn tc_replica_new<'a>(path: *mut TCString) -> *mut TCReplica {
    let storage_res = if path.is_null() {
        StorageConfig::InMemory.into_storage()
    } else {
        let path = TCString::from_arg(path);
        StorageConfig::OnDisk {
            taskdb_dir: path.to_path_buf(),
        }
        .into_storage()
    };

    let storage = match storage_res {
        Ok(storage) => storage,
        // TODO: report errors somehow
        Err(_) => return std::ptr::null_mut(),
    };

    Box::into_raw(Box::new(TCReplica {
        inner: Replica::new(storage),
        error: None,
    }))
}

/*
 * TODO:
 * - tc_replica_all_tasks
 * - tc_replica_all_task_uuids
 * - tc_replica_working_set
 * - tc_replica_get_task
 */

/// Create a new task.  The task must not already exist.
///
/// Returns the task, or NULL on error.
#[no_mangle]
pub extern "C" fn tc_replica_new_task<'a>(
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

/* - tc_replica_import_task_with_uuid
 * - tc_replica_sync
 */

/// Undo local operations until the most recent UndoPoint.
///
/// Returns -1 on error, 0 if there are no local operations to undo, and 1 if operations were
/// undone.
#[no_mangle]
pub extern "C" fn tc_replica_undo<'a>(rep: *mut TCReplica) -> i32 {
    wrap(rep, |rep| Ok(if rep.undo()? { 1 } else { 0 }), -1)
}

/// Get the latest error for a replica, or NULL if the last operation succeeded.
///
/// The returned string is valid until the next replica operation.
#[no_mangle]
pub extern "C" fn tc_replica_error<'a>(rep: *mut TCReplica) -> *const c_char {
    let rep: &'a TCReplica = rep_ref(rep);
    if let Some(ref e) = rep.error {
        e.as_ptr()
    } else {
        std::ptr::null()
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
